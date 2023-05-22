#ifndef RESPONSE_H
#define RESPONSE_H
typedef struct Response
{
    char HTTP_version[10];
    int status_code;
    char status_message[10];
    struct Header *headers;
} Response;


#include "common.h"

void free_header_request(Header *h);
void free_response(struct Response *response);
char **tokenize_command(char *cmd);
// char *modifydate(int changeday, struct tm tm);
void send_response_header(int client_sockfd, struct Response *response);
void send_response_file(int new_socket, char *url);
void set_header_and_HTTPversion(int status_code, struct Response *response);

#endif