#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include "httpd.h"

#define WWW_ROOT "/home/notoriety/Desktop/aprog2024/www_root"
#define SERVER_NAME "APROG2024 - Pedro Sousa"


/*
    Here, I decided to make an hash table to optimize long switch cases, and for a more easy implementation in the future for more
    HTTP status and contents
*/
typedef struct {
    char *hash;
    char *value;
} Hash_table_t;

Hash_table_t content_types[] = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".js", "application/javascript"},
    {".pdf", "application/pdf"},
    {".txt", "text/plain"},
    {".ico", "image/x-icon"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {NULL,NULL}};

Hash_table_t httpMessages[] = {
    {HTTP_404 , "<h1>404 - Page Not Found</h1>"},
    {NULL,NULL}
};

    //To make the code more readable when returning status code
    enum FILE_STATUS{ 
        IS_DIRECTORY,
        IS_FILE,
        NOT_FOUND
    };

//The functions used
void process_request(int client_fd, char *path, HttpRequest *req, char *status);
void send_error(int client_fd, char *status, HttpRequest *req);
char *check_path(char *path);
int is_directory(char *path);
char *get_content_type(char *path);
void sendHeader(HttpRequest *req, int client_fd, char *status, size_t fileSize, char *content);
size_t get_content_length(char *path);

int main(int argc, char const *argv[]) {
    serve_forever(3490);
    return 0;
}

void handle_request(HttpRequest *req, int client_fd) {

    if(strcmp(req->method,"GET")) return; //Close the connection if it not GET


    log_info("Request: %s", req->uri); //Logs the request to stderr

    char path[1024]; //Makes a path and concatenates with the root directory and the asked file
    strcpy(path, WWW_ROOT);
    strcat(path, req->uri);

    if (!strcmp(req->uri, "/favicon.ico")) { //Checks if it is favicon
        if (access(path, F_OK)) { //If cant open it
            sendHeader(req,client_fd,HTTP_204,0,"text/plain"); //Send a null header for 204
            return;
        }
    }

    char *status = check_path(path); //Gets if it is HTTP 200 or 404

    if (!strcmp(status, HTTP_200)) {
        process_request(client_fd, path, req, status); 
    } else {
        send_error(client_fd, HTTP_404, req);
    }
}

void send_error(int client_fd, char *status, HttpRequest *req) {

    char *message = NULL;

    for (size_t i = 0; httpMessages[i].hash != NULL; i++) { //Searchs in the hash table for the HTTP message based on the status
        if (!strcmp(status, httpMessages[i].hash)) {
            message = httpMessages[i].value;
            break;
        }
    }

    if(!message) return; //NO HTTP was found in the hash table

    size_t fileSize = strlen(message); //Calculates message size

    sendHeader(req, client_fd, status, fileSize, "text/html"); //Sends the header
    send(client_fd, message, fileSize, 0); //Sends the message
}
void process_request(int client_fd, char *path, HttpRequest *req, char *status) {

    FILE *f = fopen(path,"rb");
    size_t bytes_read;
    char buffer[1024]; // 1KB buffer

    size_t fileSize = get_content_length(path); //Gets the file size
    char *content = get_content_type(path); //Gets the content type 

    sendHeader(req, client_fd, status, fileSize, content); //Sends the header

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f))) {
        send(client_fd, buffer, bytes_read, 0); //While its reading the binary file its sending to the client
    }

    fclose(f);
}

void sendHeader(HttpRequest *req, int client_fd, char *status, size_t fileSize, char *content) {
    char header[1024];
    char date[128];

    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt); //Formats the date

    //Sending no cache to prevent problems when testing

    snprintf(header, sizeof(header),"%s %s\r\n""Date: %s\r\n""Server: %s\r\n""Content-Type: %s\r\n""Content-Length: %zu\r\n"
            "Cache-Control: no-cache\r\n""\r\n",req->http_version, status, date,SERVER_NAME,content, fileSize); //Makes the header 

    send(client_fd, header, strlen(header), 0);
}
char *check_path(char *path) {
    char *defaultFile = "index.html"; //Default file
    int lenPath = strlen(path);
    int flag = is_directory(path);

    if (flag == IS_DIRECTORY) { //If the path is a directory
        if (path[lenPath - 1] != '/') { //Checks for the '/' at the end
            strcat(path, "/"); //Adds the '/' the end if doesn't have it
        }
        strcat(path, defaultFile);//Adds the index.html at the end to search
    }

    FILE *f = fopen(path,"r");
    if(!f ) return HTTP_404; //If the file does not open does not exist

    fclose(f);

    return HTTP_200;
}


int is_directory(char *path) { //Gets the file type
    struct stat path_stat;
    if (stat(path, &path_stat)) { //THe files does not exist
        return NOT_FOUND;
    }
    return S_ISDIR(path_stat.st_mode) ? IS_DIRECTORY : IS_FILE;
}

char *get_content_type(char *path) {
    char *type = strrchr(path, '.');
    if (type) { //If the extension isnt null
        for (size_t i = 0; content_types[i].hash != NULL; i++) { //Searchs in the hash table for the content-type based on the extension
            if (!strcmp(type, content_types[i].hash)) {
                return content_types[i].value;
            }
        }
    }
    return "text/html";
}
size_t get_content_length(char *path) { //Returns the file size
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}