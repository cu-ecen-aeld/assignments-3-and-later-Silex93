/****************************************************************************
 *  Author:     Daniel Mendez
 *  Course:     ECEN 5823
 *  Project:    Assignment_5
 *
 ****************************************************************************/

/**
 * @file        aesdsocket.c
 * @brief       Source file for socker server
 *
 * @details     This file contains the function to initialize the LETIMER0 module
 *              with the following settings:
 *
 * @sources     - Beej Guide to Network Programming :https://beej.us/guide/bgnet/html/ Leveraged code from 6.1 A simple Stream Server with modifications
 *              - Linux System Programming : Chapter 10 Signals Page 342
 *

 *
 * @date        27 Sep 2023
 * @version     1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>


#define PORT "9000"
#define BUFFER_SIZE 1024
#define BACKLOG 10   // how many pending connections queue will hold
#define OUTPUT_FILE "/var/tmp/aesdsocketdata"
#define ERROR_RESULT (-1)

char * packet_buffer;

static void signal_handler(int signo) {
    printf("Signal Recieved %d \r\n",signo);
    syslog(LOG_DEBUG, "Caught signal, exiting");
    exit(EXIT_SUCCESS);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


void daemonize() {
    // Fork off the parent process
    pid_t pid = fork();

    // Exit if the fork was unsuccessful
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    // If we got a good PID, then we can exit the parent process
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);

    // Create a new SID for the child process
    pid_t sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // Change the current working directory (optional)
    // chdir("/");

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Redirect standard file descriptors to /dev/null or log files
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) {
            close(fd);
        }
    }

   
}

int main(int argc, char *argv[]) {



    int socket_fd, client_socket;
    struct addrinfo hints, *address_results, *nodes;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr;
    socklen_t sin_size;;
    //struct sigaction signal_action;
    int yes = 1;
    bool daemon_mode = false;

    openlog(NULL, 0, LOG_USER);

    // Parse command line arguments
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = true;
        printf("Daemon Mode enabled \r\n");
    }


    //Set up SIG INT handler
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot setup SIGINT!\n");
        return ERROR_RESULT;
    }

    // Setup SIG TERM Handler

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        fprintf(stderr, "Cannot setup SIGTERM!\n");
        return ERROR_RESULT;
    }

    //Remove the output file
    if (remove(OUTPUT_FILE) == 0) {
        printf("File '%s' deleted successfully.\n", OUTPUT_FILE);
    }



    //First get the available addresses on the host at port 9000
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int rc = getaddrinfo(NULL, PORT, &hints, &address_results);

    if (rc != 0) {
        //Handle Error
        fprintf(stderr, "Failed to get address info Error: %s\n", gai_strerror(rc));
        return ERROR_RESULT;
    }
    //Iterate through and bind
    for (nodes = address_results; nodes != NULL; nodes = nodes->ai_next) {
        if ((socket_fd = socket(nodes->ai_family, nodes->ai_socktype,
                                nodes->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(socket_fd, nodes->ai_addr, nodes->ai_addrlen) == -1) {
            close(socket_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(address_results);

    //Make sure bind was successful
    if (nodes == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return ERROR_RESULT;
    }
	//since bind was successful now lets start as a daemon
	if(daemon_mode){
		daemonize();
	}

    //Start listening using socket
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("failed to listen to connection");
        return ERROR_RESULT;
    }

//    signal_action.sa_handler = signal_handler;
//    sigemptyset(&signal_action.sa_mask);
//    signal_action.sa_flags = SA_RESTART;
//    if (sigaction(SIGCHLD, &signal_action, NULL) == -1) {
//        perror("sigaction");
//        exit(1);
//    }
    printf("Currently listening for connections!\n");

    while (1) {  // main accept() loop
        sin_size = sizeof their_addr;
        client_socket = accept(socket_fd, (struct sockaddr *) &their_addr, &sin_size);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *) &their_addr),
                  s, sizeof s);
        printf("server: got connection from %s \r\n", s);
        syslog(LOG_INFO, "Accepted connection from %s \r\n", s);


        
        bool end_of_packet = false;
        uint32_t packet_buffer_pos = 0;


        char buffer[BUFFER_SIZE*20];

        ssize_t bytes_received;

        //Allocate for the packet buffer
        packet_buffer = (char *) malloc(BUFFER_SIZE*20);

        while (1) {
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            end_of_packet = false;
            if (bytes_received == -1) {
                perror("recv");
                close(client_socket);
                break;
            } else if (bytes_received == 0) {
                // Connection closed by the client
                syslog(LOG_INFO, "Closed connection from %s \r\n", s);
                printf("Connection closed \r\n");
                close(client_socket);
                break;
            }
            //Iterate through the bytes received and add them to the buffer to write to the file
            for (int i = 0; i < bytes_received; i++) {
                packet_buffer[packet_buffer_pos++] = buffer[i];
                //Keep appending bytes till a newline is seen
                if (buffer[i] == '\n') {
                    packet_buffer[packet_buffer_pos] = '\0';
                    packet_buffer_pos = 0;
                    end_of_packet = true;
                    break;
                }

            }

            if (end_of_packet) {
                //Echo the packet serverside just to make sure
                printf("Incoming packet contained %s", packet_buffer);

                //Create the output file if it doesn't exist
                int log_fd = open(OUTPUT_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);

                //Append the contents of the packet to the file
                ssize_t bytes_written = write(log_fd, packet_buffer, strlen(packet_buffer));
                if (bytes_written == -1) {
                    perror("child process failed to write to file");
					free(packet_buffer);
                    close(log_fd);
                    return 1; // Return an error code
                }
                close(log_fd);
                // Check if a packet has finish being received and then send back the contents of the file
                if (!fork()) {
                    FILE *file;
                    char *tmp_buffer;
                    size_t file_size;

                    //Child Process doesn't need server socket
                    close(socket_fd);

                    file = fopen(OUTPUT_FILE, "rb");
                    if (file == NULL) {
                        perror("writer process unable to open file");
                        exit(EXIT_FAILURE);
                    }


                    // Get the file size
                    fseek(file, 0, SEEK_END);
                    file_size = ftell(file);
                    rewind(file);// Move the pointer back to the start
                    //Malloc memory to read the current file contents

                    // Allocate memory for the file contents
                    tmp_buffer = (char *) malloc(file_size);
                    if (tmp_buffer == NULL) {
                        perror("writer process failed to malloc");
                        fclose(file);
                        exit(EXIT_FAILURE);
                    }

                    // Read the entire file into the buffer
                    size_t bytes_read = fread(tmp_buffer, 1, file_size, file);
                    if (bytes_read != file_size) {
                        perror("child process failed to read file into memory");
                        fclose(file);
                        free(tmp_buffer);
                        exit(EXIT_FAILURE);
                    }
                    //Close the file
                    fclose(file);

                    //Send the file contents back to the parent
                    if (send(client_socket, tmp_buffer, file_size, 0) == -1) {
                        perror("child process failed to send file");
                    }
					free(tmp_buffer);
                    close(client_socket);
                    

                    exit(0);



                }
            }
					

        }
		
		free(packet_buffer);
        close(client_socket);  // No need for client socket anymore
       // return 0;
    }
	
    return 0;

}