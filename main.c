#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "linked_list.h"

#define SOCKET_NAME "/dev/log"
#define BUFFER_SIZE 1024
#define LOGIN_NAME_SIZE 64

int changeOwner(const char *pathName);

int main() {
    int ret, connection_socket, data_socket;
    struct sockaddr_un name;
    char buffer[BUFFER_SIZE], login[LOGIN_NAME_SIZE];
    tList list;
    listInit(&list);

    exit(1);


    if ((ret = changeOwner(SOCKET_NAME)) < 0) {
        fprintf(stderr, "WARNING: Could not change the ownership for \"%s\"\n", SOCKET_NAME);
    }

    getlogin_r(login, LOGIN_NAME_SIZE);

    unlink(SOCKET_NAME);

    /* Create local socket. */
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     */
    memset(&name, 0, sizeof(struct sockaddr_un));

    /* Bind socket to socket name. */
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(connection_socket, 20);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* This is the main loop for handling connections. */
    for (;;) {

       /* Wait for incoming connection. */
       data_socket = accept(connection_socket, NULL, NULL);
       if (data_socket == -1) {
           perror("accept");
           exit(EXIT_FAILURE);
       }

       for (;;) {

           /* Wait for next data packet. */
           ret = read(data_socket, buffer, BUFFER_SIZE);
           if (ret == -1) {
               perror("read");
               exit(EXIT_FAILURE);
           }
           break;

           /* Ensure buffer is 0-terminated. */
           buffer[BUFFER_SIZE - 1] = 0;

           /* Printf buffer*/
           printf("Log message: %s\n", buffer);


       }

       printf("Log message: %s\n", buffer);
       close(data_socket);
    }

    close(connection_socket);

    /* Unlink the socket. */

    unlink(SOCKET_NAME);

    exit(EXIT_SUCCESS);
}

int changeOwner(const char *pathName) {
    uid_t uid = getuid();
    gid_t gid = getgid();

    printf("%d %d\n", uid, gid);

    if (chown(pathName, uid, gid) < 0) {
        return -1;
    }

    return 0;
}
