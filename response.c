#include "common.h"
#include "response.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>

extern int cors;

// frees the response data structures
void free_response(struct Response *response)
{
    if (response)
    {
        if (response->headers)
            free_header_request(response->headers);
        if(response->body)
            free(response->body);
        free(response);
    }
}

char **tokenize_command(char *cmd)
{
    // GET http://127.0.0.1/home/rsh-raj/Documents/KGP/sem6/networks/Networks-Lab-Spring-2023/Assign1/Assgn-1.pdf:3000
    int index = 0;
    char temp[5000];

    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(1000 * sizeof(char));

    int cnt = 0;
    int flag = 0;
    int space = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ')
            continue;
        flag = 1;

        cnt = 0;
        if (space == 1 && cmd[i] == ' ')
            continue;
        else if (cmd[i] == ' ')
        {
            temp[cnt++] = cmd[i];
            space = 1;
            continue;
        }

        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i - 1] != '\\'))
        {
            if (cmd[i] == '\0')
                break;
            if (cmd[i] == '\\')
            {
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
            // added random
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        char **tmp = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        if (tmp == NULL)
        {
            printf("realloc failed:367\n");
            exit(1);
        }
        cmdarr = tmp;

        if (cmd[i] == '\0')
            break;
        cmdarr[index] = (char *)malloc(1000 * sizeof(char));
    }

    cmdarr[index] = NULL;
    return cmdarr;
}

char *modifydate(int changeday, struct tm tm)
{
    // time_t t = time(NULL);
    // struct tm tm = *localtime(&t);
    tm.tm_mday += changeday;
    mktime(&tm);
    char buf[50];
    strcpy(buf, asctime(&tm));

    buf[strlen(buf) - 1] = '\0';
    // printf("%s\n", buf);

    char **temp = tokenize_command(buf);

    // Now HTTP formatting
    char *final = (char *)malloc(100 * sizeof(char));
    strcpy(final, temp[0]);
    strcat(final, ", ");
    strcat(final, temp[2]);
    strcat(final, " ");
    strcat(final, temp[1]);
    strcat(final, " ");
    strcat(final, temp[4]);
    strcat(final, " ");
    strcat(final, temp[3]);
    strcat(final, " ");
    strcat(final, "IST");

    free(temp);

    // printf("Final date : %s\n", final);
    return final;
}

void send_response_header(int client_sockfd, struct Response *response)
{
    char *responseString = malloc(50);

    strcpy(responseString, response->HTTP_version);
    strcat(responseString, " ");

    char status_code[4];
    sprintf(status_code, "%d", response->status_code);
    strcat(responseString, status_code);

    strcat(responseString, " ");
    strcat(responseString, response->status_message);
    strcat(responseString, "\r\n");

    struct Header *h;
    for (h = response->headers; h; h = h->next)
    {
        char *temp = (char *)realloc(responseString, strlen(responseString) + strlen(h->name) + strlen(h->values) + 5);
        responseString = temp;

        strcat(responseString, h->name);
        strcat(responseString, ": ");
        strcat(responseString, h->values);
        strcat(responseString, "\r\n");
    }

    strcat(responseString, "\r\n");
    printf("FOLLOWING RESPONSE STRING HAS BEEN SENT TO CLIENT:\n");
    printf("%s\n", responseString);
    send(client_sockfd, responseString, strlen(responseString), 0); // send the header and HTTP version

    free(responseString);
}

void send_response_file(int new_socket, char *url)
{

    // send the file
    FILE *fp = fopen(url, "r");
    char *buffer = malloc(1025 * sizeof(char));
    int n;
    int totalBytes = 0;
    // printf("XXX\n");

    while (1)
    {   
        // printf("ABC\n");
        n = fread(buffer, 1, 1024, fp);
        // printf("ABC\n");
        if(n <= 0) break;

        send(new_socket, buffer, n, 0);
        // printf("ABC\n");
        totalBytes += n;
    }

    // printf("xxx\n");
    printf("\nTotal bytes sent: %d\n", totalBytes);
    fclose(fp);
    printf("File sent successfully!\n");
    free(buffer);
}

// new functions from here
void set_header(Response *res, char *name, char *val){
    Header *h = (Header *)malloc(sizeof(Header));
    h->name = strdup(name);
    h->values = strdup(val);

    h->next = res->headers;
    res->headers = h;
}

void set_header_and_HTTPversion(int status_code, struct Response *response)
{
    if (status_code == 200 || status_code == -1){
        strcpy(response->status_message, "OK");
        status_code = 200;
    }
    else if (status_code == 400)
        strcpy(response->status_message, "Bad Request");
    else if (status_code == 403)
        strcpy(response->status_message, "Forbidden");
    else if (status_code == 404)
        strcpy(response->status_message, "Not Found");
    else if (status_code == 302){
        strcpy(response->status_message, "Found");
    }

    strcpy(response->HTTP_version, "HTTP/1.1");
    response->status_code = status_code;

    // no default headers if redirect
    if(status_code == 302) return;

    // start setting default headers
    time_t t = time(NULL);
    struct tm tmarst = *localtime(&t);
    // header->values = modifydate(3, tmarst);
    set_header(response, "Expires", modifydate(3, tmarst));
    set_header(response, "Cache-Control", "no-store always");
    set_header(response, "Content-language", "en-us");
    set_header(response, "Connection", "keep-alive");
    // header->next = malloc(sizeof(struct Header));
    // header = header->next;
    // header->name = strdup("Cache-Control");
    // header->values = strdup("no-store always");

    // header->next = malloc(sizeof(struct Header));
    // header = header->next;
    // header->name = strdup("Content-language");
    // header->values = strdup("en-us");

    // header->next = malloc(sizeof(struct Header));
    // header = header->next;
    // header->name = strdup("Connection");
    // header->values = strdup("keep-alive");
    
    if(cors){
        set_header(response, "Access-Control-Allow-Origin", origin);
        set_header(response, "Access-Control-Allow-Methods", "GET, PUT, POST, DELETE");
        set_header(response, "Access-Control-Allow-Headers", "Content-Type, Authorization");
        // header->next = malloc(sizeof(struct Header));
        // header = header->next;
        // header->name = strdup("Access-Control-Allow-Origin");
        // header->values = strdup(origin);

        // header->next = malloc(sizeof(struct Header));
        // header = header->next;
        // header->name = strdup("Access-Control-Allow-Methods");
        // header->values = strdup("GET, PUT, POST, DELETE");

        // header->next = malloc(sizeof(struct Header));
        // header = header->next;
        // header->name = strdup("Access-Control-Allow-Headers");
        // header->values = strdup("Content-Type, Authorization");
    }
}


Response *new_response(){
    Response *temp = (Response *)malloc(sizeof(Response));
    temp->body = NULL;
    temp->headers = NULL;
    temp->status_code = -1;
    temp->status_message[0] = '\0';
    
    return temp;
}

// send the response including the body
// generally used by user when sending custom response
// the headers in this will be set by the user
void send_response(Response *res, int sock){

    Header *user_header = res->headers;
    res->headers = NULL;

    // set default headers and HTTP version
    // set some default set of headers on the basis of the status code
    // default headers CORS, content-length, content-type, 
    set_header_and_HTTPversion(res->status_code, res);

    // set the content length and text format headers
    int len = res->body ? strlen(res->body) : 0;
    char str[10];
    sprintf(str, "%d", len);

    // set the headers
    set_header(res, "Content-Length", str);
    set_header(res, "Content-Type", "text/*");
    
    // check if the user has already set them, if yes, over ride the default
    // very important step
    int index = 0;
    while(user_header){
        // check if this header exists in default headers
        index = 0;
        Header *h = res->headers;
        Header *temp = user_header->next;
        while(h){
            if(strcmp(user_header->name, h->name) == 0){
                h->values = user_header->values;
                index = 1;

                // free the header
                user_header->next = NULL;
                free_header_request(user_header);
                break;
            }

            h = h->next;
        }
        if(index == 0){
            // means the goven header is not in the default headers
            user_header->next = res->headers;
            res->headers = user_header;
        }

        user_header = temp;;
    }

    // send the headers, including version and status message
    send_response_header(sock, res);
    // send the body if it is not present
    if(res->body) send(sock, res->body, strlen(res->body), 0); 
}

void CORS_enable(char *address){
    cors = 1;
    strcpy(origin, address);
}


