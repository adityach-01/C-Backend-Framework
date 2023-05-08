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

// frees the response data structures
void free_response(struct Response *response)
{
    if (response)
    {
        if (response->headers)
            free_header_request(response->headers);
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

void set_header_and_HTTPversion(int status_code, struct Response *response)
{
    if (status_code == 200)
        strcpy(response->status_message, "OK");
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

    struct Header *header = malloc(sizeof(struct Header));
    header->next = NULL;
    response->headers = header;

    if(status_code == 302) return;

    header->name = strdup("Expires");
    // header->values = strdup("Thu, 01 Dec 1994 16:00:00 GMT");
    time_t t = time(NULL);
    struct tm tmarst = *localtime(&t);
    header->values = modifydate(3, tmarst);

    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Cache-Control");
    header->values = strdup("no-store always");

    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Content-language");
    header->values = strdup("en-us");

    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Connection");
    header->values = strdup("keep-alive");
    
    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Access-Control-Allow-Origin");
    header->values = strdup("http://localhost:8080, http://localhost:8081");

    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Access-Control-Allow-Methods");
    header->values = strdup("GET, PUT, POST, DELETE");

    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Access-Control-Allow-Headers");
    header->values = strdup("Content-Type, Authorization");
    header->next = NULL;
}