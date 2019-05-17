#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>

#include "linked_list.h"

#define SOCKET_NAME "/dev/log"
#define BUFFER_SIZE 1024
#define LOGIN_NAME_SIZE 64
#define PENDING_CONNECTIONS_QUEUE 10

int changeOwner(const char *pathName);
int changeMod(const char *pathName);
void signalHandler(int sig);

tList list;
extern int argc;

int main(int argc, char *argv[]) {
    int ret, connection_socket, data_socket;
    struct sockaddr_un name;
    char buffer[BUFFER_SIZE];
    listInit(&list);

    if (signal(SIGINT, signalHandler) == SIG_ERR)
        fprintf(stderr, "WARNING: Can't catch SIGINT\n");


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
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    /* Change ownership for the socket */
    // if ((ret = changeOwner(SOCKET_NAME)) < 0) {
    //     fprintf(stderr, "WARNING: Could not change the ownership for \"%s\"\n", SOCKET_NAME);
    // }

    /* Bind socket to socket name. */
    ret = bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (changeMod(SOCKET_NAME) < 0) {
        fprintf(stderr, "WARNING: Could not change mod for %s\n", SOCKET_NAME);
    }

    /*
    * Prepare for accepting connections. The backlog size is set
    * to 10. So while one request is being processed other requests
    * can be waiting. Note if a connection request
    * arrives when the queue is full, the client may receive an error with
    * an indication of ECONNREFUSED.
    */
    ret = listen(connection_socket, PENDING_CONNECTIONS_QUEUE);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* This is the main loop for handling connections. */
    for (;;) {
        memset(buffer, 0, BUFFER_SIZE);
        /* Wait for incoming connection. */
        data_socket = accept(connection_socket, NULL, NULL);
        if (data_socket == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        /* Wait for next data packet. */
        ret = read(data_socket, buffer, BUFFER_SIZE);
        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Ensure buffer is 0-terminated. */
        buffer[BUFFER_SIZE - 1] = 0;

        /* If the logging message is empty */
        if (buffer[0] == '\0')
            continue;

        /* Printf buffer*/
        printf("Log message: %s", buffer);
        insertFirst(&list, buffer);

        close(data_socket);
    }

    close(connection_socket);

    /* Unlink the socket. */
    unlink(SOCKET_NAME);

    destroyList(&list);

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

int changeMod(const char *pathName) {
    int mod = S_IFSOCK; // It is a socket;
    mod |= S_IRWXU; // Mask for file owner permissions(r+w+x);
    mod |= S_IRGRP | S_IWGRP; // Mask for group permissions(r+w);
    mod |= S_IROTH | S_IWOTH; // Mask for other permissions(r+w);

    if (chmod(pathName, mod) < 0)
        return -1;
    return 0;
}

void signalHandler(int sig) {
    if (sig == SIGINT) {
        printf("\n");
        print(&list);
        tElem *mostPopular = getMostPopular(&list);
        if (mostPopular) {
            char login[LOGIN_NAME_SIZE];
            getlogin_r(login, LOGIN_NAME_SIZE);
            printf("%d --> %s", mostPopular->counter, mostPopular->messageBody);
        }
        else {
            printf("Nothing to print\n");
        }
        destroyList(&list);
        printf("The END!\n");
        exit(EXIT_SUCCESS);
    }
}
