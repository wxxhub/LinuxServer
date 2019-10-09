#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[]) {

    if (argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 0;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;

    inet_pton(AF_INET, ip, &address.sin_family);

    address.sin_port = htons(port);

    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listen_fd, 5);
    assert(ret != -1);

    struct sockaddr_in client_address;
    socklen_t client_addr_length = sizeof(client_address);
    int conn_fd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addr_length);

    if (conn_fd < 0) {
        printf("errno is: %d", errno);
        close(listen_fd);
    }

    char buf[1024];
    fd_set read_fds;
    fd_set exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);

    while (true) {
        memset(buf, '\0', sizeof(buf));

        FD_SET(conn_fd, &read_fds);
        FD_SET(conn_fd, &exception_fds);

        ret = select(conn_fd + 1, &read_fds, NULL, &exception_fds, NULL);

        if (ret < 0) {
            printf("selection failure\n");
            break;
        }

        if (FD_ISSET(conn_fd, &read_fds)) {
            ret = recv(conn_fd, buf, sizeof(buf) - 1, 0);

            if (ret <= 0) {
                break;
            }

            printf("get %d bytes of normal data: %s\n", ret, buf);
        } else if(FD_ISSET(conn_fd, &exception_fds)) {
            ret = recv(conn_fd, buf, sizeof(buf) - 1, MSG_OOB);

            if (ret <= 0) {
                break;
            }

            printf("get %d bytes of oob data: %s\n", ret, buf);
        }
    }

    close(conn_fd);
    close(listen_fd);
    return 0;
}