#include "globals.h"

#include <unistd.h>

std::unordered_map<std::string, std::string> user_list;   // maps usernames to passwords
std::string FS_MAX_ENCRYPTED_TEXT = std::to_string(std::numeric_limits<unsigned int>::max());
std::unordered_map<std::string,std::unordered_map<unsigned int, unsigned int>> sessionInfo;
std::unordered_map<unsigned int, rw_lock> blockLock; 
unsigned int sessionID = 0;
std::mutex guard;

/***************************************************
 * Required: A string
 * Modifies: Nothing
 * Effects:  Returns false if any of the pathnames
 *           in the string are > FS_MAXFILENAME
 ***************************************************/
bool isBadPath(std::string pathname) {
    unsigned int stringLoc = 0;
    string x;
    size_t pos;
    while(1) {
        pos = pathname.find('/', stringLoc);
        if (pos == string::npos) {
            return false;
        }
        x = pathname.substr(stringLoc, pos - stringLoc);
        if (x.length() > 59) { //too long
            return true;
        }
        stringLoc = pos + 1;
    }
    return false;
}

/********************************************************
 * Requires: A string
 * Modifies: Nothing
 * Effects: Returns false if all elements of the string
 *          are not digits.
 ********************************************************/
bool isNumber(std::string num) {
    return std::all_of(num.begin(), num.end(), ::isdigit);
}

/*************************************************************************
 * Required:    username and password are well formed                                                            
 * Modifies:    user_list                                                 
 * Effects:     Adds username and password pairs into user list
 *************************************************************************/
void read_userlist(){
	std::string username = "";
	std::string password = "";
    while ( std::cin >> username ) {
    	std::cin >> password;
    	user_list[username] = password;
	}
}// end read_userlist()

/*************************************************************************
 * Required: null terminated c-string                                                        
 * Modifies:                                                  
 * Effects: On error, returns 0. Errors include 
                1. username contains non-alphanumeric characters
                2. <space> is not followed by a nullbyte
                3. <username> does not begin with <space>
            Else, returns the index of the character in buf after <space>,
                e.g. <username> <size><NULL>
                     ^^^^^^^^^^^^ 
 *************************************************************************/
unsigned int find_request_size_start_loc(const char * buf, unsigned int len){
    // returns the start of where <size> in the cleartext header should begin
    // if the cleartext header is well-formed. returns 0 on errors
    assert(buf[len] == '\0');
    for (unsigned int i = 0; i < len; i++){
        if (buf[i] == ' ') {
            if (i == 0) {
                _(
                    err_cout("Username has a <space> in it");
                )
                return 0;
            }

            if (buf[i+1] == '\0') {
                _(
                    err_cout("Username is followed by <NULL>.");
                )
                return 0;
            }

            return i+1;
        }
        if(!isalnum(buf[i])){
            _(
                err_cout("Found a non-alphanumeric character.");
            )
            return 0;
        }
    } // end for
    _(
        err_cout("Got through whole message without reading a string?");
    )
    return 0;
} // end find_request_size_start_loc()

/*************************************************************************
 * Required: null terminated c-string                                                        
 * Modifies:                                                
 * Effects: On error, returns empty string, "". Errors include:
                1. Nondigit in <size>
                2. <size> > MAX(unsigned int); i.e., integer overflow
            else, return the size of the encrypted text as a string
 *************************************************************************/
std::string extract_encrypted_text_size(const char *buf, unsigned int start, unsigned int end) {
    assert(buf[end] == '\0');
    std::string size;
    for(unsigned int i = start; i < end; i++){
        if (!isdigit(buf[i])){
            _(
                err_cout("Found a not-digit");
            )
            return "";
        }
        size += buf[i];
    }

    if(size.size() == FS_MAX_ENCRYPTED_TEXT.size() ) {
        if( size > FS_MAX_ENCRYPTED_TEXT ) {
            _(
                err_cout("Size is too large.");
            )
            return "";
        }
    }
    if(size.size() > FS_MAX_ENCRYPTED_TEXT.size()) {
        return "";
    }

    return size;
} // end extract_encrypted_text_size()

/*************************************************************************
 * Requires:    
 * Modifies:    blockLock
 * Effects:     Traverses the file system and populates blockLock
 *************************************************************************/
bool populate() {
    std::queue<populate_data> fillQueue;
    
    // push the root onto the queue
    populate_data temp;
    temp.diskBlock = 0;
    temp.blockType = 'i';
    fillQueue.push(temp);

    // BFS through the filesystem
    while (!fillQueue.empty()) {
        temp = fillQueue.front();
        fillQueue.pop();
        blockLock[temp.diskBlock]; // initializes lock for the thing that was just popped off
        _(
            std::cout << "Used diskblocks:" << std::to_string(temp.diskBlock) << std::endl;
        )

        if (temp.blockType == 'i') { // disk block is an fs_inode
            fs_inode read;
            disk_readblock(temp.diskBlock, &read);
            if (read.type == 'f') { // inode read is a file
                for (uint32_t i = 0; i < read.size; ++i) {
                    if (read.blocks[i] == 0) {
                        _(
                            err_cout("ERROR: In populate(), existing filesystem used block 0 for file data.");
                        )
                        return false;
                    }
                    blockLock[read.blocks[i]]; // initializes lock for 
                }
            } else if(read.type == 'd') { //inode read is a directory
                temp.blockType = 'd';
                for (uint32_t i = 0; i < read.size; ++i) {
                    if (read.blocks[i] == 0) {
                        _(
                            err_cout("ERROR: In populate(), existing file system used block 0 for blocks");
                        )
                        return false;
                    }
                    temp.diskBlock = read.blocks[i];
                    fillQueue.push(temp);
                } // end for
            } // end if-else if
        } else if (temp.blockType == 'd') { // disk block is of type fs_direntry[FS_DIRENTRIES]
            fs_direntry read[FS_DIRENTRIES];
            disk_readblock(temp.diskBlock, &read);
            temp.blockType = 'i';
            for (unsigned int i = 0; i < FS_DIRENTRIES; ++i) {
                if (read[i].inode_block != 0) {
                    temp.diskBlock = read[i].inode_block;
                    fillQueue.push(temp);
                } // end if
            } //end for
        }// end if-elseif
    } // end while
    return true;
} // end populate()

// in /, pathname: you/me/myself
// traverse for you/me
// blocknum for me
// me, myself
// in you, pathname: me/myself
/*************************************************************************
 * Required:    pathname (should not begin with /), disk_block to inode you
 *                  want to start reading from, and string specifying user
 *                  block must be locked before entering
 * Modifies:    Nothing
 * Effects:     returns disk block of the inode you are searching for, or 
 *              FS_DISKSIZE or FS_DISKSIZE + 1 as error
 *              FS_DISKSIZE + 1 -> tried to access a disk_block you do not own
 *              FS_DISKSIZE     -> disk_block you were searching for could not
 *                                      could not be found
 *************************************************************************/
unsigned int traverse(std::string pathname, unsigned int disk_block, std::string user, bool writeLock) {
    _(
        err_cout("FLAG: In Traverse...");
    )
    assert(user != "");
    if (pathname[0] == '/') {
        _(
            err_cout("ERROR: The pathname passed into traverse begins with a /");
        )
        blockLock[disk_block].reader_unlock();
        return FS_DISKSIZE;
    }
    fs_inode base;
    disk_readblock(disk_block, &base);

    if (strcmp("", base.owner) != 0 && strcmp(user.c_str(), base.owner) != 0) {
        _(
            assert(disk_block != 0);
            err_cout("ERROR: You are not the owner we are looking for.");
        )
        blockLock[disk_block].reader_unlock();
        return FS_DISKSIZE + 1; 
    }

    std::string target = pathname.find('/') > 0 ? pathname.substr(0, pathname.find('/')) : pathname ;
    if(target.size() > FS_MAXFILENAME) {
        _(
            err_cout("ERROR: Searched for a pathname that's too big");
        )
        blockLock[disk_block].reader_unlock();
        return FS_DISKSIZE;
    }
    pathname = (target == pathname) ? pathname : pathname.substr(pathname.find('/') + 1);

    for (unsigned int i = 0; i < base.size; i++) {
        fs_direntry subject[FS_DIRENTRIES];
        disk_readblock(base.blocks[i], &subject);
        for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
            if (subject[j].inode_block && strcmp(subject[j].name, target.c_str()) == 0) {
                if (target == pathname) {
                    _(
                        err_cout("FLAG: In traverse; we found what we were looking for! Returning from traverse.");
                    )
                    // TODO: check this.
                    if(writeLock) {
                        blockLock[subject[j].inode_block].writer_lock();

                    } else {
                        blockLock[subject[j].inode_block].reader_lock();
                    }
                    blockLock[disk_block].reader_unlock();
                    return subject[j].inode_block;
                } else {
                    blockLock[subject[j].inode_block].reader_lock();
                    blockLock[disk_block].reader_unlock();
                    return traverse(pathname, subject[j].inode_block, user, writeLock); // TODO: check to make sure this isr right
                } // end if-else
            } // end if
        } // end for
    } // end for
    _(
        err_cout("ERROR: Couldn't find inode block within traverse.");
    )
    blockLock[disk_block].reader_unlock(); // TODO: check this;
    return FS_DISKSIZE;
}

/*************************************************************************
 * Required:    username, sequence number
 * Modifies:    sessionInfo
 * Effects:     Adds username -> session/sequence to sessionInfo
                Returns sessionID
 *************************************************************************/
unsigned int start_session(std::string username, unsigned int sequence) {
    guard.lock();
    sessionInfo[username][sessionID] = sequence;
    unsigned int temp = sessionID++;
    guard.unlock();
    return temp;
} // end start_session()

/*************************************************************************
 * Required:    username, pathname, type of direntry
 * Modifies:    some inodeBlock
 * Effects:     Creates a direntry at pathname
 *************************************************************************/
bool create_inode(std::string username, std::string pathname, char type) {
    if(pathname == "/") {
        _(
            err_cout("ERROR: Trying to make root.");
        )
        return false;
    }
    size_t lastSlash = pathname.find_last_of('/');
    if (lastSlash == std::string::npos) {
        _(
            err_cout("ERROR: Malformed pathname? Couldn't find a / anywhere in create_inode.");
        )
        return false;
    }
    std::string directory;
    if (lastSlash == 0) {
        // creating within root
        directory = "";
    } else {
        directory = pathname.substr(1, lastSlash - 1);
    }
    std::string target = pathname.substr(lastSlash + 1);

   if (target.size() > FS_MAXFILENAME) {
        // Size of filename to be created exceeds defined limits
        _(
            err_cout("Error: In create_inode(), size of filename exceeds limits.");
        )
        return false;
    }
    guard.lock();
    unsigned int newBlock = get_free_blocks();
    if(newBlock == FS_DISKSIZE) {
        _(
            err_cout("Error: no more free blocks to write into.");
        )
        guard.unlock();
        return false;
    }
    blockLock[newBlock];
    guard.unlock();

    // retrieve diskblock number corresponding to the parent directory
    unsigned int parentInodeBlock;
    if(directory == "") {
        parentInodeBlock = 0;
        blockLock[parentInodeBlock].writer_lock();
    } else {
        blockLock[0].reader_lock();
        parentInodeBlock = traverse(directory, 0, username, true);
    }
    // Invariant: Writer lock is held for parentInode,
    // unless we called traverse and couldn't find it.

    if (parentInodeBlock >= FS_DISKSIZE) {
        _(
            err_cout("ERROR: Couldn't find the inode_block.");
        )
        guard.lock();
        auto it = blockLock.find(newBlock);
        blockLock.erase(it);
        guard.unlock();
        return false;
    } else {
        // Found parent directory
        // Read information from that diskBlock
        fs_inode parentInode;
        disk_readblock(parentInodeBlock, &parentInode);
        assert(parentInode.size <= FS_MAXFILEBLOCKS);
        
        if (strcmp(parentInode.owner, username.c_str()) != 0 && strcmp("", parentInode.owner) != 0) {
            _(
                err_cout("ERROR: These are not the files you are looking for, devious one.");
            )
            guard.lock();
            auto it = blockLock.find(newBlock);
            blockLock.erase(it);
            guard.unlock();
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        } else if (parentInode.type == 'f') {
            _(
                err_cout("ERROR: Tried to create a file under a file. IGNORE");
            )
            guard.lock();
            auto it = blockLock.find(newBlock);
            blockLock.erase(it);
            guard.unlock();
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        }

        // Diskblock # holding the free direntry
        unsigned int freeBlockNumberHoldingDirEntry = FS_MAXFILEBLOCKS;

        // Index of the free direntry in the diskblock
        unsigned int freeDirEntryPos = FS_DIRENTRIES;

        fs_direntry parentDirEntries[FS_DIRENTRIES];
        fs_direntry foundDirEntries[FS_DIRENTRIES];
        bool foundEmpty = false;
        for (unsigned int i = 0; i < parentInode.size; i++) {
            // iterate through parents blocks and go through
            // each block's direntries
            disk_readblock(parentInode.blocks[i], &parentDirEntries);
            for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
                // for each direntry, find the first free disk block
                if (!foundEmpty && parentDirEntries[j].inode_block == 0) {
                    // found a direntry with inode block 0, which indicates a free direntry 
                    foundEmpty = true;
                    freeBlockNumberHoldingDirEntry = parentInode.blocks[i];
                    freeDirEntryPos = j;
                    for (unsigned int k = 0; k < FS_DIRENTRIES; k++) {
                        if (parentDirEntries[k].inode_block != 0) {
                            // copy contents of parentDirEntries to foundDirEntries
                            strcpy(foundDirEntries[k].name, parentDirEntries[k].name);
                            foundDirEntries[k].inode_block = parentDirEntries[k].inode_block;
                        } else {
                            foundDirEntries[k].inode_block = 0;
                            strcpy(foundDirEntries[k].name, "");
                        } // end if-else
                    } // end for
                } else if (parentDirEntries[j].inode_block != 0 && strcmp(parentDirEntries[j].name, target.c_str()) == 0) {
                    _(
                        err_cout("ERROR: Found duplicate names while trying to create inode. Name: " + target);
                    )
                    guard.lock();
                    auto it = blockLock.find(newBlock);
                    blockLock.erase(it);
                    guard.unlock();
                    blockLock[parentInodeBlock].writer_unlock();
                    return false;
                } // end if-else if
            } // end for
        } // end for

        if(parentInode.size == FS_MAXFILEBLOCKS && freeBlockNumberHoldingDirEntry == FS_MAXFILEBLOCKS) {
            _(
                err_cout("ERROR: parentInode has 124 blocks and has no empty space for another subdirectory/file.");
            )
            guard.lock();
            auto it = blockLock.find(newBlock);
            blockLock.erase(it);
            guard.unlock();
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        }

        // create the new inode and write it to disk
        fs_inode newInode;
        newInode.type = type;
        strcpy(newInode.owner, username.c_str());
        newInode.size = 0; 

        if (freeBlockNumberHoldingDirEntry == FS_MAXFILEBLOCKS){
            // no available fs_direntries within this block left
            // must allocate new block for parentInode.
            guard.lock();
            unsigned int secondFreeBlock = get_free_blocks(); 
            if (secondFreeBlock == FS_DISKSIZE) {
                _(
                    err_cout("ERROR: No more free blocks.");
                )
                // free the first free block
                auto del = blockLock.find(newBlock);
                blockLock.erase(del);
                guard.unlock();
                blockLock[parentInodeBlock].writer_unlock();
                return false;
            }
            blockLock[secondFreeBlock];
            guard.unlock();
            disk_writeblock(newBlock, &newInode);

            fs_direntry dirEntryArray[FS_DIRENTRIES];

            for (unsigned int i = 0; i < FS_DIRENTRIES; i++) {
                // set all the other block numbers to 0
                dirEntryArray[i].inode_block = 0;
                strcpy(dirEntryArray[i].name, "");
            }
 
            strcpy(dirEntryArray[0].name, target.c_str());
            dirEntryArray[0].inode_block = newBlock;

            // write the new direntries array onto disk
            disk_writeblock(secondFreeBlock, &dirEntryArray);

            // update the parentInode's size and blocks array
            parentInode.blocks[parentInode.size++] = secondFreeBlock;
            disk_writeblock(parentInodeBlock, &parentInode); 
        } else {
            // found a free direntry within an existing block of parent's inode
            strcpy(foundDirEntries[freeDirEntryPos].name, target.c_str());
            foundDirEntries[freeDirEntryPos].inode_block = newBlock;

            disk_writeblock(newBlock, &newInode);

            disk_writeblock(freeBlockNumberHoldingDirEntry, &foundDirEntries);
        } //end if-else
    } //end other if-else

    _(
        std::cout << "Got to the end of create_inode()!" << std::endl;
    )
    blockLock[parentInodeBlock].writer_unlock();
    return true;
} // end create_inode()

/*************************************************************************
 * Required:    username, pathname
 * Modifies:    some inodeBlock
 * Effects:     Deletes a direntry at pathname
 *************************************************************************/
bool delete_inode(std::string username, std::string pathname) {
    _(
        err_cout("Within delete_inode(). Pathname is:" + pathname);
    )
    if(pathname == "/") {
        // trying to delete root
        _(
            err_cout("ERROR: Trying to delete root.");
        )
        return false;
    } else {
        std::string directory;
        // Process the pathname and remove the first '/'
        size_t lastSlash = pathname.find_last_of('/');
        if (lastSlash == std::string::npos) {
            _(
                err_cout("ERROR: lastSlash not found within pathname while calling delete_inode().");
            )
            return false;
        }
        if(lastSlash == 0) {
            // deleting something within root. i.e., /tmp
            directory = "";
        } else {
            directory = pathname.substr(1, lastSlash - 1);
        } // end if-else

        std::string target = pathname.substr(lastSlash + 1);

        unsigned int parentInodeBlock;
        if(directory == "") {
            parentInodeBlock = 0;
            blockLock[parentInodeBlock].writer_lock();
        } else {
            blockLock[0].reader_lock();
            parentInodeBlock = traverse(directory, 0, username, true);
        }
        if (parentInodeBlock >= FS_DISKSIZE) {
            _(
                err_cout("ERROR: Deleting pathname not in disk, or inode not owned by username.");
            )
            return false;
        }
        _(
            err_cout("FLAG: Back in our delete function(). Traverse returned:" + std::to_string(parentInodeBlock));
        )
        int deleteParentIdx = -1;
        int deleteDirEntryIndex = -1;
        int count = 0;

        // read parentInodeBlock into parentInode
        fs_inode parentInode;
        disk_readblock(parentInodeBlock, &parentInode);
        if (parentInode.type != 'd') {
            _(
                err_cout("ERROR: In delete, parentInode is not a directory.");
            )
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        }
        if (strcmp(parentInode.owner, username.c_str()) != 0 && strcmp("", parentInode.owner) != 0) {
            _(
                err_cout("ERROR: These are not the owners to the files you are looking for.");
            )
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        }
        
        // array of direntries
        fs_direntry parentDirEntries[FS_DIRENTRIES];
        
        for (unsigned int i = 0; i < parentInode.size; i++) {
            // iterate through parents blocks and go through each block's direntries to find one with the same name as our target
            
            // read parent's inode pointers in parent.blocks into an array of direntries
            disk_readblock(parentInode.blocks[i], &parentDirEntries);
            count = 0;
            for (unsigned int j = 0; j < FS_DIRENTRIES; j++) {
                // iterate through each direntry to find the node we want to delete
                if (parentDirEntries[j].inode_block != 0) {
                    // counts the number of unused disk blocks in this direntry
                    ++count;
                    if (strcmp(parentDirEntries[j].name, target.c_str()) == 0) {
                        // found a match, but keep iterating through this block's direntries
                        deleteParentIdx = i;
                        deleteDirEntryIndex = j;
                    } // end if
                } // end if
            } // end for
            if (deleteParentIdx != -1 && deleteDirEntryIndex != -1) {
                // if we found a match in the previous block's direntries, break.
                break;
            }
        } // end for

        if (deleteParentIdx == -1 && deleteDirEntryIndex == -1) {
            // could not find the file we were trying to delete. return error
            _(
                err_cout("ERROR: In FS_DELETE(), did not find file to delete.");
            )
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        }
 
        fs_inode victim;
        blockLock[parentDirEntries[deleteDirEntryIndex].inode_block].writer_lock(); // lock the victim to make sure there are no readers or writers in there

        disk_readblock(parentDirEntries[deleteDirEntryIndex].inode_block, &victim);

        if (strcmp(victim.owner, username.c_str()) != 0) {
            _(
                err_cout("ERROR: User tried to delete a file/directory that didn't belong to them.");
            )
            blockLock[parentDirEntries[deleteDirEntryIndex].inode_block].writer_unlock();
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        }

        if (victim.type == 'd' && victim.size > 0) {
            _(
                err_cout("ERROR: Trying to delete a nonempty directory. size:" + std::to_string(victim.size));
            )
            blockLock[parentDirEntries[deleteDirEntryIndex].inode_block].writer_unlock();
            blockLock[parentInodeBlock].writer_unlock();
            return false;
        } else if (victim.type == 'f') {
            // delete the file blocks that are used
            guard.lock();
            for (unsigned int i = 0; i < victim.size; i++) {
                // free the blocks
                auto delete_it = blockLock.find(victim.blocks[i]);
                blockLock.erase(delete_it);
            } // end for
            guard.unlock();
        } // end if-elseif

        // free victim block
        guard.lock();
        blockLock[parentDirEntries[deleteDirEntryIndex].inode_block].writer_unlock();
        auto delete_it = blockLock.find(parentDirEntries[deleteDirEntryIndex].inode_block);
        blockLock.erase(delete_it);
        guard.unlock();
        if (count == 1) {
            // parentInode has a block with only one direntry in it and we're deleting that direntry
            _(
                err_cout("FLAG: ParentInode has a block with only one fs_direntry in it and we're deleting it. Decrementing the size of parentInode.");
            )

            // swap parentInode's deleteParentIdx with the last one used
            for (uint32_t i = deleteParentIdx; i < parentInode.size -1; ++i) {
                std::swap(parentInode.blocks[i], parentInode.blocks[i + 1]);
            }
            
            // update deleteParentIdx because we just swapped it to the last element
            deleteParentIdx = parentInode.size - 1;

            // decrement parentInode's block size
            --parentInode.size;

            // commit the changes
            disk_writeblock(parentInodeBlock, &parentInode);
            guard.lock();
            auto delete_it = blockLock.find(parentInode.blocks[deleteParentIdx]);                                                                            
            blockLock.erase(delete_it);
            guard.unlock();
        } else {
            // update dir_entries for parentInode
            parentDirEntries[deleteDirEntryIndex].inode_block = 0;
            strcpy(parentDirEntries[deleteDirEntryIndex].name, "");

            disk_writeblock(parentInode.blocks[deleteParentIdx], &parentDirEntries);
        } // end if-else

        blockLock[parentInodeBlock].writer_unlock();
        return true;
    } //end if-else`
} // end delete_inode()

/*************************************************************************
 * Required:    username, pathname, block of file you wish to read, array to pass data into
 * Modifies:    readData
 * Effects:     Reads data of the file at block blockNum and passes it back into readData
 *************************************************************************/
std::string read_file(std::string username, std::string pathname, unsigned int blockNum) {
    if(pathname == "/"){
        _(
            err_cout("ERROR: User is trying to read the root directory, which is not a file.");
        )
        return "";
    }
    blockLock[0].reader_lock(); // lock's root before going into traverse.
    unsigned int targetInodeBlock = traverse(pathname.substr(1), 0, username, false);
    if (targetInodeBlock >= FS_DISKSIZE) {
        _(
            err_cout("ERROR: Reading a block that couldn't be found or we don't own.");
        )
        return "";
    }

    fs_inode targetInode;
    disk_readblock(targetInodeBlock, &targetInode);
    if (targetInode.type != 'f' || blockNum >= targetInode.size || strcmp(username.c_str(), targetInode.owner) != 0) {
        _(
            err_cout("ERROR: targetInode is not a file, couldn't locate, or does not belong to user");
        )
        blockLock[targetInodeBlock].reader_unlock();
        return "";
    }
    unsigned int dataBlock = targetInode.blocks[blockNum];
    assert(dataBlock != 0);

    char readData[FS_BLOCKSIZE];
    disk_readblock(dataBlock, &readData);

    // send the response
    blockLock[targetInodeBlock].reader_unlock(); // unlocks inode when you're done with
    return std::string(readData, FS_BLOCKSIZE); 
} // end read_file()

/*************************************************************************
 * Required:    username, pathname, block of file you wish to write, data you wish to write. 
                    writeData should be exactly 512 bytes
 * Modifies:    inodeBlock
 * Effects:     writes data from writeData into file at block blockNum
 *************************************************************************/
bool write_file(std::string username, std::string pathname, unsigned int blockNum, std::string writeData) {
    assert(blockNum < FS_MAXFILEBLOCKS);
    assert(writeData.size() == FS_BLOCKSIZE);
    if (pathname[0] != '/') {
        _(
            err_cout("ERROR: Pathname invalid, should start by reading from root.");
        )
        return false;
    }
    if(pathname == "/") {
        _(
            err_cout("ERROR: trying to write to root. Bad.");
        )
        return false;
    }
    blockLock[0].reader_lock(); // lock's root before going into traverse
    unsigned int targetInodeBlock = traverse(pathname.substr(1), 0, username, true);
    
    if (targetInodeBlock >= FS_DISKSIZE) {
        _(
            err_cout("ERROR: Writing to a block that couldn't be found or we don't own.");
        )
        return false;
    }

    fs_inode targetInode;
    disk_readblock(targetInodeBlock, &targetInode);
    if (targetInode.type != 'f' || blockNum > targetInode.size || strcmp(username.c_str(), targetInode.owner) != 0) {
        _(
            err_cout("ERROR: In write_file, can't write because it's not a file, blockNum is not correct, or we're not the owner.");
        )
        blockLock[targetInodeBlock].writer_unlock();
        return false;
    }

    unsigned int targetBlock;

    if (blockNum == targetInode.size) {
        // "Growing" the file
        guard.lock();
        targetBlock = get_free_blocks();
        if (targetBlock == FS_DISKSIZE) {
            _(
                err_cout("ERROR: Within write_file, no more free blocks.");
            )
            blockLock[targetInodeBlock].writer_unlock();
            guard.unlock();
            return false;
        }
        blockLock[targetBlock];
        guard.unlock();
    } else {
        // Writing to an existing block
        guard.lock();
        if (blockLock.find(targetInode.blocks[blockNum]) == blockLock.end()) {
            _(
                err_cout("ERROR: blockLock not found?");
            )
            guard.unlock();
            blockLock[targetInodeBlock].writer_unlock();
            return false;
        }
        guard.unlock();
        targetBlock = targetInode.blocks[blockNum];
    }

    assert(targetBlock != 0);

    disk_writeblock(targetBlock, (void *) writeData.c_str());

    if (blockNum == targetInode.size) {
        // update the targetInode with the newly allocated block
        targetInode.blocks[blockNum] = targetBlock;
        ++targetInode.size;
        disk_writeblock(targetInodeBlock, &targetInode);
    }
    blockLock[targetInodeBlock].writer_unlock();
    return true;
} // end write_file()

/*************************************************************************
 * Required:    Unencrypted string message
 * Modifies:    
 * Effects:     Error checks message and calls corresponding helper function
 *************************************************************************/
std::string parse_message(std::string username, std::string message, unsigned int size_decrypted) {
    _(
        err_cout("Starting parse_message with message:" + message);
    )
    //to keep track of position of where to start parsing
    unsigned int stringLoc = 0;
    //read in request, session number, and sequence number
    unsigned int argsRead = 0;
    unsigned int i = 0;
    std::string request, pathname, blockNumString, writeData, readData;
    unsigned int session;
    unsigned int sequence;
    unsigned int blockNum;
    bool hitNull = false;
    char type;
    // Reads in <request> <session> <sequence><NULL>
    while (argsRead < 3 && i < message.size()) {
        if (message[i] == ' ' || message[i] == '\0') {
            if (message[i] == '\0') {
                if (hitNull) {
                    _(
                        err_cout("ERROR: In parse_message, more than one <NULL>.");
                    )
                    return ""; 
                } else {
                    hitNull = true;
                }
            }
            if (argsRead == 0) {
                // extracts <request>
                request = message.substr(0, i);
            } else if (argsRead == 1) {
                // extracts <session> 
                std::string tempString = message.substr(stringLoc, i - stringLoc);
                if (!isNumber(tempString)) {
                    _(
                        err_cout("ERROR: In parse_message, stoul failed while extracting <session>.");
                    )
                    return "";
                }
                session = std::stoul(message.substr(stringLoc, i - stringLoc));
            } else if (argsRead == 2) {
                // extracts <sequence>
                std::string tempString = message.substr(stringLoc, i - stringLoc);
                if (!isNumber(tempString)) {
                    _(
                        err_cout("ERROR: In parse_message, stoul failed while extracting <sequence>.2");
                    )
                    return "";
                }
                sequence = std::stoul(message.substr(stringLoc, i - stringLoc));
            } // end if-elseif-elseif
            stringLoc = i + 1;
            ++argsRead;
        } // end if
        ++i;
    } // end while

    if (argsRead != 3) { 
        _(
            err_cout("ERROR: Parse_message failed to parse.");
        )
        return "";
    }

    //assign request to int for switch
    unsigned int switchNum;

    _(
        err_cout("extracted_request:"+request);
        err_cout("extracted_session:"+std::to_string(session));
        err_cout("etracted_sequence:"+std::to_string(sequence));
    );

    if (request == "FS_SESSION") {
        if(!hitNull || stringLoc != message.size()) {
            _(
                err_cout("ERROR: In parse_message, couldn't find <NULL>.");
            )
            return "";
        }
        switchNum = 0;
    } else if (request == "FS_CREATE") {
        switchNum = 1;
    } else if (request == "FS_DELETE") {
        switchNum = 2;
    } else if (request == "FS_READBLOCK") {
        switchNum = 3;
    } else if (request == "FS_WRITEBLOCK") {
        switchNum = 4;
    } else {
        _(
            err_cout("ERROR: In parse_message, request was invalid:"+request);
        )
        return ""; 
    }

    std::string response_message = "";

    if (switchNum) {
        guard.lock();
        // check if session number is valid for user, if so, update sequence number
        if (sessionInfo[username].find(session) == sessionInfo[username].end()) {
            // session doesnt exist, invalid request
            _(
                err_cout("ERROR: Session not found within user database. The username was:" + username + ", and session was:" + std::to_string(session));
            )
            guard.unlock();
            return "";
        }
        else if (sessionInfo[username][session] >= sequence) {
            // check if new sequence number is greater than the old one 
            _(
                err_cout("ERROR: Input sequence number was too small.");
                err_cout("ERROR: System sequence:" + std::to_string(sessionInfo[username][session]) + ", and input sequence:" + std::to_string(sequence));
            )
            guard.unlock();
            return "";
        } else {
            assert(sessionInfo[username][session] < sequence);
            sessionInfo[username][session] = sequence;
            guard.unlock();
            size_t pos;
            if (switchNum == 2) {   //delete only has pathname
                //find from sequence number to null (takes in pathname)
                if(message.find(' ', stringLoc) != std::string::npos) {
                    _(
                        err_cout("ERROR: Malformed request for FS_DELETE. Pathname has a space in it.");
                    )
                    return "";
                }
                pos = message.find('\0', stringLoc);
                
                if (pos == std::string::npos) {
                    _(
                        err_cout("ERROR: Malformed request for FS_DELETE. Couldn't extract <pathname>.");
                    )
                    return "";
                }
                else if(pos != message.size() - 1){
                    _(
                        err_cout("ERROR: Malformed request for FS_DELETE. Multiple <NULL>");
                    )
                    return "";
                };
            } else {    //other requests have other parameters
                //find from sequence number to next parameter beginning
                pos = message.find(' ', stringLoc);
                if (pos == std::string::npos) {
                    _(
                    err_cout("ERROR: Malformed request. Couldn't extract <pathname>.");
                    )
                    return "";
                }
            }
            
            pathname = message.substr(stringLoc, pos - stringLoc);
            if (pathname[0] != '/' || pathname[pathname.size() - 1] == '/' || pathname.size() > FS_MAXPATHNAME) {
                _(
                    err_cout("ERROR: Malformed request. Pathname doesn't begin with /, ends with a /, or pathname too long.");
                )
                return "";
            }
            for (unsigned int pathnameCheck = 0; pathnameCheck < pathname.size() - 1; ++pathnameCheck) {
                if(pathname[pathnameCheck] == '/' && pathname[pathnameCheck + 1] == '/') {
                    _(
                        err_cout("ERROR: Pathname error. two '/' characters in a row.");
                    )
                    return "";
                } else if (pathname[pathnameCheck] == '\0' || pathname[pathnameCheck + 1] == '\0') {
                    _(
                        err_cout("ERROR: Pathname error. Multiple <NULL>.");
                    )
                }
            }
            if(isBadPath(pathname)) {
                _(
                    err_cout("ERROR: This pathname has sections that are WAY too long.");
                )
                return "";
            }

            _(
                err_cout("pathname:" + pathname);
            )

            stringLoc += pathname.length() + 1; // points to a character after the space
            if (switchNum == 3 || switchNum == 4) { // READBLOCK or WRITEBLOCK
                if (stringLoc >= message.size()) {
                    _(
                        err_cout("ERROR: Malformed request. Message too short.");
                    )
                    return "";
                }
                pos = message.find('\0', stringLoc);
                if (pos == std::string::npos) {
                    _(
                        err_cout("ERROR: Read/Write malformed request.");
                    )
                    return "";
                }
                blockNumString = message.substr(stringLoc, pos - stringLoc);
                if (!isNumber(blockNumString)) {
                     _(
                        std::cout << "ERROR: In parse_message, stoul failed, meaning the blockNum contains non-numeric characters in it." << std::endl;
                    )
                    return "";
                }
                stringLoc += blockNumString.length() + 1;
                blockNum = stoul(blockNumString);
                if (blockNum >= FS_MAXFILEBLOCKS) {
                    _(
                        err_cout("ERROR: In parse_message, request blockNum for read/write too large.");
                    )
                    return "";
                }
            } // end if
        } // end if-elseif-else
    } // end if

    response_message = std::to_string(session) + " " + std::to_string(sequence) + '\0' ;
    //switch for request type
    switch(switchNum) {
        case 0: // FS_SESSION
            // make sure session number is 0
            if (session) {
                _(
                    err_cout("ERROR: In parse_message, session is nonzero.");
                )
                return "";
            }
            response_message = std::to_string(start_session(username, sequence)) + " " + std::to_string(sequence) + '\0';
            break;
        case 1: // FS_CREATE
            if (stringLoc != (message.size() - 2) || message[stringLoc - 1] != ' ')  {
                _(
                    err_cout("ERROR: Malformed request for FS_CREATE.");
                )
                return "";
            }
            type = message[stringLoc]; 
            if (message[message.size() - 1] != '\0' || (type != 'd' && type != 'f')) {
                _(
                    err_cout("ERROR: Malformed request for FS_CREATE. Should be <type={'f','d'}><NULL>");
                )
                return "";
            }
            if (!create_inode(username, pathname, type)) {
                _(
                    err_cout("ERROR: Create_inode threw an error.");
                )
                return "";
            }
            break;
        case 2: // FS_DELETE
            if (stringLoc != message.size() || message[stringLoc-1] != '\0') {
                _(
                    err_cout("ERROR: Malformed request for FS_DELETE.");
                )
                return "";
            }
            if (!(delete_inode(username, pathname))) {
                _(
                    err_cout("ERROR: Delete_inode threw an error.");
                )
                return "";
            }
            break;
        case 3: // FS_READBLOCK
            if (stringLoc != message.size() || message[stringLoc - 1] != '\0') {
                _(
                    err_cout("ERROR: Malformed read request.");
                )
                return "";
            }
            readData = read_file(username, pathname, blockNum);
            if(readData == "") {
                _(
                    err_cout("ERROR: Read file threw an error.");
                )
                return "";
            }
            response_message += readData;
            break;
        case 4: // FS_WRITEBLOCK
            if (stringLoc >= message.size() || message[stringLoc - 1] != '\0') {
                _(
                    err_cout("ERROR: Write_file is too short or does not have an ending null before writedata");
                )
                return "";
            }
            writeData = message.substr(stringLoc, size_decrypted - stringLoc);
            if (writeData.size() != FS_BLOCKSIZE ) {
                _(
                    err_cout("ERROR: Data to write is not of 512 bytes.");
                )
                return "";
            } 
            if (!(write_file(username, pathname, blockNum, writeData))) {
                _(
                    err_cout("ERROR: Write_file threw an error.");
                )
                return "";
            }
            break;
        default:
            _(
                err_cout("ERROR: User command not recognized. Please enter one of the following: FS_SESSION, FS_CREATE, FS_DELETE, FS_READBLOCK, FS_WRITEBLOCK.");
            )
            return "";
            break;
    } // end switch
    return response_message;
} // end parse_message()


/*************************************************************************
 * Effects:     Returns the first free, nonzero disk block
 *************************************************************************/
unsigned int get_free_blocks() {
    for (unsigned int i = 1; i < FS_DISKSIZE; i++) {
        if (blockLock.find(i) == blockLock.end()) {
            return i;
        }
    }
    return FS_DISKSIZE;
} // end get_free_blocks();

/*************************************************************************
 * Effects:     Sends server response 
 *************************************************************************/
bool send_server_response(int sock, const char * str, const unsigned int len) {
    int nsent = 0;
    int ntosend = len;
    while (ntosend != 0) {
        /* Send any unsent data bytes remaining */
        int rval = send(sock, str + nsent, ntosend, MSG_NOSIGNAL);    // returns the # of bytes sent
        if (rval == -1) {
            // ERROR
            _(
                err_cout("Failure during message transmission from server to client.");
                err_cout(strerror(errno));
            )
            return false;
        }

        ntosend -= rval;
        nsent += rval;
    }
    return true;
} // end send_server_response()




_(
/*************************************************************************
 * Effects:     Debugging output
 *************************************************************************/
    void err_cout(std::string err_msg){
        cout_lock.lock();
        for (unsigned int i = 0; i < err_msg.size(); i++){
            if (err_msg[i] == '\0') {
                std::cout << "<NULL>";
            } else {
                std::cout << err_msg[i];
            }
        }
        std::cout << std::endl;
        cout_lock.unlock();
    } 

    void err_cout(const char * str, unsigned int len) {
        cout_lock.lock();
        for(unsigned int i = 0; i < len; i++){
            if (str[i] == '\0') {
                std::cout << "<NULL>";
            } else {
                std::cout << str[i];
            }
        }
        std::cout << std::endl;
        cout_lock.unlock();
    }
);