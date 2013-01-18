/**
 * Author: Copy crazy_fire from his CSDN BLOG 
 * Email: (mine)lusknight@gmail.com
 * Last modified: 2012-08-01 10:28
 * Filename: HttpSession.h
 * Description: 处理HTTP会话的头文件 
 */
#ifndef _HTTPSESSION_H_
#define _HTTPSESSION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>

#define RECV_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 1050000
#define URI_SIZE 128
#define TIME_BUFFER_SIZE 40

#define TIME_OUT_SEC 10
#define TIME_OUT_USEC 0
#define URI_TOO_LONG 414
#define ALLOW "Allow:GET"
#define SERVER "Server:Mutu(0.1 Alpha)/Linux"

#define FILE_OK 200
#define FILE_NOT_FOUND 404

#define FILE_MAX_SIZE 1048576

/* 以下为Http 会话中的函数声明 */

int http_session(int* connect_socket, struct sockaddr_in* client_addr);
int is_http_protocol(char* msg_from_client);
char* get_uri(char* request_header, char* uri_buffer);
int get_uri_status(char *uri);
char *get_mime_type(char *uri); 
int get_file_disk(char *uri, unsigned char *file_buffer); 
int reply_normal_information(unsigned char *send_buffer, unsigned char *file_buffer, int file_size, char *mime_type); 
char *get_time_str(char *time_buf);
#endif
