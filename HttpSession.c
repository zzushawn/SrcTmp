/**
 * Author: Copy crazy_fire from his CSDN BLOG 
 * Email: (mine)lusknight@gmail.com
 * Last modified: 2012-08-01 10:28
 * Filename: HttpSession.c
 * Description: 处理HTTP会话 
 */

#include "HttpSession.h"

int http_session(int* connect_socket, struct sockaddr_in* client_addr) {

    /* 服务器端接收缓冲区 */
    char recv_buffer[RECV_BUFFER_SIZE + 1];
    /* 服务器端发送缓冲区 */
    unsigned char send_buffer[SEND_BUFFER_SIZE + 1];

    unsigned char file_buffer[FILE_MAX_SIZE + 1];

    memset(recv_buffer, '\0', sizeof(recv_buffer));
    memset(send_buffer, '\0', sizeof(send_buffer));
    memset(file_buffer, '\0', sizeof(file_buffer));

    /* 存储客户端的URI请求 */
    char uri_buffer[URI_SIZE + 1];
    memset(uri_buffer, '\0', sizeof(uri_buffer));

    int maxfd = *connect_socket + 1;
    fd_set read_set;
    FD_ZERO(&read_set);

    /**
     * 时间 time_out 有三种情况
     *  1、为0 调用立即返回
     *  2、为NULL 时select调用就阻塞，直到有文件描述符就绪
     *  3、为正整数时，就是一般定时器
     */
    struct timeval time_out;
    time_out.tv_sec = TIME_OUT_SEC;
    time_out.tv_usec = TIME_OUT_USEC;

    int flag = 1;
    int res = 0;
    int read_bytes = 0;
    int send_bytes = 0;
    int file_size = 0;
    char* mime_type;
    int uri_status;
    /**
     * 设置对应fd_set文件描述符集的位为1，即将连接的socket对应与fd_set的位为1
     */
    FD_SET(*connect_socket, &read_set);

    while(flag) {

        res = select(maxfd, &read_set, NULL, NULL, &time_out);

        switch(res) {
            case -1:
                close(*connect_socket);
                return -1;
                break;
            case 0:
                continue;
                break;
            default:
                /**
                 * 测试connect_socket 是否可读
                 */
                if(FD_ISSET(*connect_socket, &read_set)) {
                    memset(recv_buffer, '\0', sizeof(recv_buffer));
                    if((read_bytes = recv(*connect_socket, recv_buffer, RECV_BUFFER_SIZE, 0)) == 0) {
                        /* client close the socket */
                        return 0;
                    } else if(read_bytes > 0) {
                        /* 判断是不是HTTP协议 */
                        if(is_http_protocol(recv_buffer) == 0) {
                            close(*connect_socket);
                            return -1;
                        } else {
                            memset(uri_buffer, '\0', sizeof(uri_buffer));
                            /** get the uri from http request head */
                            if(get_uri(recv_buffer, uri_buffer) == NULL) {
                                uri_status = URI_TOO_LONG;
                            } else {
                                uri_status = get_uri_status(uri_buffer);
                                switch(uri_status) {
                                    case FILE_OK:
                                        mime_type = get_mime_type(uri_buffer);
                                        file_size = get_file_disk(uri_buffer, file_buffer);
                                        send_bytes = reply_normal_information(send_buffer, file_buffer, file_size, mime_type);
                                        break;
                                    case FILE_NOT_FOUND:
                                        //send_bytes = set_error_information(send_buffer, FILE_NOT_FOUND);
                                        break;
                                    default:
                                        break;
                                }
                                send(*connect_socket, send_buffer, send_bytes, 0);
                            }
                        }
                    }
                }
        }
    }

    return 0;
}

int is_http_protocol(char *msg_from_client) 
{
    int index = 0;
    while(msg_from_client[index] != '\0' && msg_from_client[index] != '\n') {
        index++;
    }
    //printf("%s", msg_from_client + index - 10);
    /* 原作者把这个地方的偏移量错写成了10 导致运行出错 
     * 应该是回退9个字符
     */
    if(strncmp(msg_from_client + index - 9, "HTTP/", 5) == 0) {
        return 1;
    }

    return 0;
}

char *get_uri(char *req_header, char *uri_buffer) 
{

    int index = 0;
    while((req_header[index] != '/') && (req_header[index] != '\0')) {
        index++;
    }

    int base = index;
    while( ((index - base) < URI_SIZE) && (req_header[index] != ' ') && (req_header[index] != '\0')) {
        index++;
    }
    if((index - base) >= URI_SIZE) {
        return NULL;
    }
    /**
     * 如果URI没有具体文件名，只有路径，则重定向到index.html这个文件
     */
    if((req_header[index - 1] == '/') && (req_header[index] == ' ')) {
        strcpy(uri_buffer, "index.html");
        return uri_buffer;
    }
    strncpy(uri_buffer, req_header + base + 1, index - base -1);
    return uri_buffer;
}

int get_uri_status(char *uri) {

    if(access(uri, F_OK) == -1) {
        fprintf(stderr, "File: %s not found.\n", uri);
        return FILE_NOT_FOUND;
    }
    if(access(uri, R_OK) == -1) {
        fprintf(stderr, "File: %s not found.\n", uri);
        return FILE_NOT_FOUND;
    }
    return FILE_OK;
}

char *get_mime_type(char *uri) {

    int len = strlen(uri);
    int dot = len - 1;
    while( dot >= 0 && uri[dot] != '.') {
        dot--;
    }
    if(dot == 0) {
        return NULL;
    }
    if(dot < 0) {
        return "text/html";
    }
    dot++;
    int type_len = len - dot;
    char *type_off = uri + dot;
    switch(type_len) {
        case 4:  
            if(!strcmp(type_off, "html") || !strcmp(type_off, "HTML"))  
            {  
                return "text/html";  
            }  
            if(!strcmp(type_off, "jpeg") || !strcmp(type_off, "JPEG"))  
            {  
                return "image/jpeg";  
            }  
            break;  
        case 3:  
            if(!strcmp(type_off, "htm") || !strcmp(type_off, "HTM"))  
            {  
                return "text/html";  
            }  
            if(!strcmp(type_off, "css") || !strcmp(type_off, "CSS"))  
            {  
                return "text/css";  
            }  
            if(!strcmp(type_off, "png") || !strcmp(type_off, "PNG"))  
            {  
                return "image/png";  
            }  
            if(!strcmp(type_off, "jpg") || !strcmp(type_off, "JPG"))  
            {  
                return "image/jpeg";  
            }  
            if(!strcmp(type_off, "gif") || !strcmp(type_off, "GIF"))  
            {  
                return "image/gif";  
            }  
            if(!strcmp(type_off, "txt") || !strcmp(type_off, "TXT"))  
            {  
                return "text/plain";  
            }  
            break;  
        case 2:  
            if(!strcmp(type_off, "js") || !strcmp(type_off, "JS"))  
            {  
                return "text/javascript";  
            }  
            break;  
        default:        /* unknown mime type or server do not support type now*/  
            return "NULL";  
            break;  
    }

    return NULL;
}

int get_file_disk(char *uri, unsigned char *file_buffer) {

    int read_count = 0;
    int fd = open(uri, O_RDONLY);
    if(fd == -1) {
        perror("open() in get_file_disk http_session.c");
        return -1;
    }
    unsigned long st_size;
    struct stat st;
    if(fstat(fd, &st) == -1) {
        perror("stat() in get_file_disk HttpSession.c");
        return -1;
    }
    st_size = st.st_size;
    if(st_size > FILE_MAX_SIZE) {
        fprintf(stderr, "the file %s is too large.\n", uri);
        return -1;
    }

    if((read_count = read(fd, file_buffer, FILE_MAX_SIZE)) == -1) {
        perror("read() in get_file_disk http_session.c");
        return -1;
    }
    //printf("file %s size : %lu, read %d\n", uri, st_size, read_count);

    return read_count;
}

int reply_normal_information(unsigned char *send_buffer, unsigned char *file_buffer, int file_size, char *mime_type) {

    char *str = "HTTP/1.1 200 OK\r\nServer:Mutu/Linux(0.1)\r\nDate:";
    int index = strlen(str);
    memcpy(send_buffer, str, index);

    char time_buf[TIME_BUFFER_SIZE];
    memset(time_buf, '\0', sizeof(time_buf));
    str = get_time_str(time_buf);
    int len = strlen(time_buf);
    memcpy(send_buffer + index, time_buf, len);
    index += len;

    len = strlen(ALLOW);
    memcpy(send_buffer + index, ALLOW, len);
    index += len;

    memcpy(send_buffer + index, "\r\nContent-Type:", 15);
    index += 15;
    len = strlen(mime_type);
    memcpy(send_buffer + index, mime_type, len);
    index += strlen(mime_type);

    memcpy(send_buffer + index, "\r\nContent-Type:", 17);
    index += 17;
    char num_len[8];
    memset(num_len, '\0', sizeof(num_len));
    sprintf(num_len, "%d", file_size);
    len = strlen(num_len);
    memcpy(send_buffer + index, num_len, len);
    index += len;

    /* 开始输出消息体 */
    memcpy(send_buffer + index, "\r\n\r\n", 4);
    index += 4;

    memcpy(send_buffer + index, file_buffer, file_size);
    index += file_size;

    return index;
}

char *get_time_str(char *time_buf) {

    time_t now_sec;
    struct tm *time_now;
    if(time(&now_sec) == -1) {
        perror("time() in HttpSession.c");
        return NULL;
    }
    if((time_now = gmtime(&now_sec)) == NULL) {
        perror("localtime in HttpSession.c");
        return NULL;
    }

    char *str_ptr = NULL;
    if((str_ptr = asctime(time_now)) == NULL) {
        perror("asctime in HttpSession.c");
        return NULL;
    }
    strcat(time_buf, str_ptr);
    return time_buf;
}
