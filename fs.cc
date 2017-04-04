#include "globals.h"
#include <thread>

void serve_request(int sock) {
    char buf[MSGSIZE+1];
     // number of bytes read in by recv
    unsigned int nread = 0;

    while (1) {
        // receive one byte at a time from the bytesream
        int num_read = recv(sock, buf + nread, 1, 0);  
        assert(num_read == 1);
        nread += num_read;
        if (buf[nread - 1] == '\0'){
            break;
        }

        if (nread > FS_MAXUSERNAME + FS_MAX_ENCRYPTED_TEXT.size() + 2) {
            // read in too many number of bytes for cleartext message
            _(
                err_cout("ERROR, client request is malformed.");
            )
            close(sock);
            return;
        }
    } // end while

    _(
        std::string temp_str = "cleartext header:" + std::string(buf, nread-1);
        err_cout(temp_str);
    )

    unsigned int cleartext_size_start = find_request_size_start_loc(buf, nread - 1);  
    if (cleartext_size_start == 0) {
        // error in the cleartext header
        _(
            err_cout("ERROR in cleartext header"); 
        )
        close(sock);
        return;
    }

    std::string username = std::string(buf, buf + cleartext_size_start - 1);

    if (user_list.find(username) == user_list.end()){
        // username is invalid
       _(
            err_cout("username is invalid: " + username); 
        )
        close(sock);
        return;
    }

    std::string encrypted_text_size = extract_encrypted_text_size(buf, cleartext_size_start, nread - 1);
    if (encrypted_text_size == "") {
        // error in <size> portion of cleartext header
        _(
            err_cout("error in <size> portion of cleartext header.");
        )
        close(sock);
        return;
    }

    unsigned int ciphertext_size = stoul(encrypted_text_size);

    // Read the ciphertext
    nread = 0;
    std::string ciphertext = "";
    while(nread != ciphertext_size){
        nread += recv(sock, buf, MSGSIZE, 0); 
        if (nread < 0) {
            // ERROR
            _(
                err_cout("ERROR: Something bad happened when reading in ciphertext.");
            )
            close(sock);
            return;
        }
        ciphertext.append(buf, nread);
    }


    unsigned int size_decrypted = 0;

    const char * password = user_list[username].c_str();

    char* result = (char*) fs_decrypt(password , (void *) ciphertext.c_str(), ciphertext_size, &size_decrypted);

    if (result == nullptr) {
        // failed to decrypt ciphertext
        _(
            err_cout("failed to decrypt."); 
        )
        close(sock);
        return;
    }
    assert(size_decrypted > 0);

    // output for checking stuff...
    _(
        err_cout(result, size_decrypted);
    )

    std::string request_header = std::string(result, size_decrypted);
    std::string response_str = parse_message(username, request_header, size_decrypted);
    delete[] result;

    if (response_str != "") {
        _(
            err_cout("response_str:[" + response_str+"]");
        )
        
        unsigned int size_encrypted;
        const char *send_response = (char *) fs_encrypt(user_list[username].c_str(), (void *) response_str.c_str(), response_str.size(), &size_encrypted);
        std::string clear_response = std::to_string(size_encrypted);

        if (!send_server_response(sock, clear_response.c_str(), clear_response.size() + 1)) {
            // Error: Something happened during message transmission
            _(
                err_cout("ERROR: Something bad happened during message transmission.");
            )
            delete[] send_response;
            result = nullptr;
            close(sock);
            return;
        }
        if (!send_server_response(sock, send_response, size_encrypted)) {
            // Error: Something happened during message transmission
            _(
                err_cout("ERROR: Something bad happened during message transmission.");
            )
            delete[] send_response;
            send_response = nullptr;
            close(sock);
            return;
        }
        delete[] send_response;
    }
    // close the connection for the request
    close(sock);   
}

static void usage(char *argv[]) {
    fprintf(stderr, 
            "Usage: %s <hostname> <port>   (client)\n"
            "       %s [ port ]            (server)\n", 
            argv[0], argv[0]);
    exit(EXIT_FAILURE);
}

static void handle_error(bool condition, const char *msg) {
    if (condition) {
        perror(msg);
        exit(1);
    }
}

static void handle_gai_error(int rc) {
    if (rc) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }
}

static int get_socket(struct addrinfo *addr) {
    int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol); //todo: wi_passiveats ai_protocol
    handle_error(sock < 0, "creating socket");
    return sock;
}

static struct addrinfo *get_server_addrinfo(const char *port) {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;                            // TODO: whats ai_passive

    addrinfo *ai;
    int rc = getaddrinfo(nullptr, port, &hints, &ai);
    handle_gai_error(rc);

    return ai;
}

static int run_server(const char *port) {   
    int rc;

    addrinfo *ai = get_server_addrinfo(port);

    int listen_sock = get_socket(ai);

    int yes = 1;
    rc = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    handle_error(rc != 0, "setsockopt failed");

    rc = bind(listen_sock, ai->ai_addr, ai->ai_addrlen);
    handle_error(rc != 0, "bind failed");

    freeaddrinfo(ai);

    rc = listen(listen_sock, 10); // 10 requests in the queue
    handle_error(rc != 0, "listen failed");

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    rc = getsockname(listen_sock, (struct sockaddr *)&addr, &len);
    handle_error(rc != 0, "getsockname failed");

    unsigned int port_number = ntohs(addr.sin_port);

    std::cout << "\n@@@ port " << port_number << std::endl;

    while (true) {
        // TODO: For multithreaded, create thread; remember to detach
        int sock = accept(listen_sock, nullptr, 0); // accept a new request
        std::thread myThread(serve_request, sock);
        myThread.detach();
    }   // end while


    assert(0);
    close(listen_sock);

    return 0;
}

int main (int argc, char *argv[]) {
    const char *port = "0";

    if (argc > 1) {
        if (argc == 2) {
            port = argv[1];
        } else {
            // ERR
            usage(argv);
        }
    }

    read_userlist();

    // BFS through exisiting filesystem
    if(!populate()){
        _(
            err_cout("ERROR: In populate(). Returning from main.");
        )
        return 0;
    } 

    return run_server(port);
} // end main()
