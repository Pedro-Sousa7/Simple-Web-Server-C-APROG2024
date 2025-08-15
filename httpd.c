
#include "httpd.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>


void log_info(const char *format, ...) 
{
    // Get the current time
    time_t now = time(NULL);
    char ts[20];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Print the timestamp
    fprintf(stderr, "[%s] ", ts);

    // Handle variadic arguments and print the message
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    // Add a newline
    fprintf(stderr, "\n");
}


int start_server(int port)
{
    int server_fd;
    int option = 1;
    struct sockaddr_in address;

    // create socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { //Verifies if there is an error
        perror ("socket() error");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); //Starts the socket

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror ("bind() error");
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
    if (listen(server_fd, 100) != 0 ) {
        perror("listen() error");
        exit(1);
    }

    return server_fd;
}

/*
    This function parses an HTTP request buffer and fills the provided HttpRequest structure.
    It does not return any value since the HttpRequest structure is passed by reference.
*/

void parse_request(char *request_buffer, HttpRequest *req) 
{
    char *lines[20] = {NULL};
    int line_count = 0;

   char *line = strtok(request_buffer, "\r\n"); // Gets the first instance until the "\r\n"

    while (line) {
        lines[line_count++] = line; // Stores the current line in the array *lines and increments the line_count
        line = strtok(NULL, "\r\n"); // Retrieves the next line
        if (line_count > 19) break; // Stops the loop if the line_count exceeds the maximum size of the array *lines
    }

    //for(size_t i=0; lines[i] && i<20; i++) printf("--> %s\n", lines[i]);

    // parse request line

    /*
        The lines[0] accesses the first value of the request and its METHOD URI HTTP_VERSION (ex: "GET / HTTP/1.1")
    */
    req->method = strtok(lines[0], " "); //stores the method in a struct (ex: GET)
    req->uri = strtok(NULL, " "); //Stores the URI (ex: /)
    req->http_version = strtok(NULL, "\r\n"); //Stores the version (ex: HTTP/1.1)

    // parse headers

    req->header_count = 0; //Initializes the value from the header_count
    for (size_t i = 0; lines[i]; i++) {
        char *line = lines[i]; // Retrieves each line from the *lines array to parse headers
        char *colon = strchr(line, ':'); // Finds the colon that separates the header key from the value (ex: "Host: 127.0.0.1:3490")
        if (!colon) continue; // If there's no colon, it's not a valid header, so skip the line

        *colon = '\0'; // Terminates the line string at the colon
        colon++; // Moves the pointer to the start of the value part after the colon
        while (*colon == ' ' || *colon == '\t') colon++; // Skips any spaces or tabs in the value

        req->headers[req->header_count].key = line; // Stores the header key
        req->headers[req->header_count].value = colon; // Stores the header value
        req->header_count++; // Increments the header count after storing the header
    }
    //printf("Version - %s\n",req->http_version);
}


void serve_forever(int PORT)
{
    int clientfd;
    char buffer[2048];    
    int n;
    
    int server_fd = start_server(PORT);
    log_info("Server started: 127.0.0.1:%d", PORT);    

    // ACCEPT connections
    while (1) {
        struct sockaddr_in clientaddr;
        socklen_t addrlen;
        int client_fd = accept (server_fd, (struct sockaddr *) &clientaddr, &addrlen);;
        if (client_fd == -1) continue;
        // log client connection

        char request_buffer[8192];
        int n = read(client_fd, request_buffer, sizeof(request_buffer));

        if (n == -1) {
            perror("read error");
        } else if (n == 0) {
            perror("client disconnected");
        } else {
            HttpRequest req;
            parse_request(request_buffer, &req); // Parse the request
            handle_request(&req, client_fd);    // Serve the file or 404
        }

        close(client_fd);
    }

    close(server_fd);
}


