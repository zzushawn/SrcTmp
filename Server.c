/**
 * Author: Copy crazy_fire from his CSDN BLOG 
 * Email: (mine)lusknight@gmail.com
 * Last modified: 2012-08-12 20:06
 * Filename: Server.c
 * Description: 测试网络I/O事件的，主要想测试libevent 或者 epoll 或者 select
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "HttpSession.h"
#define PORT 8080
#define BACKLOG 200

int init_socket(int*,  struct sockaddr_in*);

int main(int argc, char** argv) {

    int listen_socket;
    int connect_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    bzero(&client_addr, sizeof(struct sockaddr_in));

    /** 初始化监听socket */
    init_socket(&listen_socket, &server_addr);

    socklen_t addrlen = sizeof(struct sockaddr_in);
    pid_t pid;

    while(1) {

        connect_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &addrlen);
        pid = fork();
        
        if(pid > 0) {
            /* 主进程关闭连接的socket 继续循环保证其他连接请求*/
            close(connect_socket);
            continue;
        } else if (pid == 0) {
            /** 
             * 从进程关闭绑定的socket 
             * 处理HTTP会话的进程在这个if语句结束后即退出
             * 不会一直存在内存里
             */
            close(listen_socket);

            /* 处理HTTP会话 */
            if(http_session(&connect_socket, &client_addr) == -1) {
                perror("HttpSession() error in Server.c");
                shutdown(connect_socket, SHUT_RDWR);
                exit(EXIT_FAILURE);
            }
            shutdown(connect_socket, SHUT_RDWR);
            exit(EXIT_SUCCESS);
        } else {
            perror("fork() error in Server.c");
            exit(EXIT_FAILURE);
        }
    } 

    /* 关闭绑定的socket 结束server的主进程程序 */
    shutdown(listen_socket, SHUT_RDWR);
    return 0;
}

int init_socket(int* listen_socket, struct sockaddr_in* server_addr) {

    if((*listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        perror("socket() error in init_socket.c");
        return -1;
    }

    int opt = SO_REUSEADDR;
    /* 设置socket参数 */
    setsockopt(*listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /** 设置server addr的参数 */
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    bind(*listen_socket, (struct sockaddr* )server_addr, sizeof(struct sockaddr_in));

    listen(*listen_socket, BACKLOG);

    return 0;
}
