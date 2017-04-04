# Makefile for EECS 482 project 4
GCC			= g++
VERSION		= -std=c++11 

FLAGS		= -Werror -Wall -ldl -pthread

DEBUG		= -DDEBUG

GLOBALS		= globals.cc
RWLOCK 		= rw_lock.cc
EXEC		= fs.cc
OUTPUT		= fs  

LIBSERVER	= libfs_server.o
LIBCLIENT	= libfs_client.o 

TARGETS 	= fs 

SPEC		= test-spec-client.cc

ALB 		= test-notyours1.cc
HOW 		= test-changing-blocks.cc
SAM 		= test-path-length.cc

fs: $(GLOBALS) $(EXEC)
	$(GCC) $(EXEC) $(GLOBALS) $(RWLOCK) $(LIBSERVER) $(FLAGS) $(VERSION) -o $(OUTPUT)
debug:
	$(GCC) $(EXEC) $(GLOBALS) $(RWLOCK) $(LIBSERVER) $(FLAGS) $(VERSION) $(DEBUG) -o $(OUTPUT)

albert:
	$(GCC) $(ALB) $(LIBCLIENT) $(FLAGS) $(VERSION) -o albert
howie:
	$(GCC) $(HOW) $(LIBCLIENT) $(FLAGS) $(VERSION) -o howie
sam:
	$(GCC) $(SAM) $(LIBCLIENT) $(FLAGS) $(VERSION) -o sam
clean:
	rm -f $(TARGETS) 


