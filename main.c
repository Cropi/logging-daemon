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
#include <errno.h>

#include "linked_list.h"

#define SOCKET_NAME "/dev/log"
#define BUFFER_SIZE 1024
#define LOGIN_NAME_SIZE 64
#define PENDING_CONNECTIONS_QUEUE 10

int changeOwner(const char *pathName);
int changeMod(const char *pathName);
void signalHandler(int sig);
void evaluateResults(tList *list, int argc, char *argv[]);
int printResultsToFile(char *fileName, char *message);

int isOver = 0;

int main(int argc, char *argv[]) {
    int ret, connection_socket, data_socket;
    struct sockaddr_un name;
    char buffer[BUFFER_SIZE];
    tList list;
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

    while ( !isOver ) {
        memset(buffer, 0, BUFFER_SIZE);
        /* Wait for incoming connection. */

        struct timeval tv;
        int activity;
        fd_set readfds;

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(connection_socket, &readfds);

        //wait for an activity on one of the sockets
        activity = select( connection_socket + 1 , &readfds , NULL , NULL , &tv);

        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }
        if (activity == 0) {
            continue;
        }

        if (isOver)
            break;
        if (FD_ISSET(connection_socket, &readfds))
        {
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
            for(int i = 1; i < argc; i++) {
                if (printResultsToFile(argv[i], buffer) < 0)
                    fprintf(stderr, "Could not save data to %s\n", argv[i]);
            }
            insertFirst(&list, buffer);

            close(data_socket);
        }
    }


    evaluateResults(&list, argc, argv);

    close(connection_socket);
    unlink(SOCKET_NAME);

    exit(EXIT_SUCCESS);
}

void evaluateResults(tList *list, int argc, char *argv[]) {
    printf("\n");
    // print(&list);
    tElem *mostPopular = getMostPopular(list);
    if (mostPopular) {
        char login[LOGIN_NAME_SIZE];
        getlogin_r(login, LOGIN_NAME_SIZE);
        printf("%d --> %s", mostPopular->counter, mostPopular->messageBody);
    }
    else {
        printf("Nothing to print\n");
    }
    printf("The END!\n");


    char count[8];
    sprintf(count, "%d", mostPopular->counter);
    for(int i = 1; i < argc; i++) {
        if (printResultsToFile(argv[i], count) < 0)
            fprintf(stderr, "Could not save data to %s\n", argv[i]);
        if (printResultsToFile(argv[i], " --> ") < 0)
            fprintf(stderr, "Could not save data to %s\n", argv[i]);
        if (printResultsToFile(argv[i], mostPopular->messageBody) < 0)
            fprintf(stderr, "Could not save data to %s\n", argv[i]);
    }
}

int printResultsToFile(char *fileName, char *message) {
    // Only appends to a file specified by fileName, DOES NOT rewrites it
    FILE *f = fopen(fileName, "a+");

    if (f == NULL) {
        return -1;
    }

    fprintf(f, "%s", message);

    fclose(f);
    return 0;
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
        isOver = 1;
    }
}
