// added by wliu, for windows socket programming
#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>
#include <string.h>

#define SERVER_PORT 5432
#define MAX_PENDING 5
#define MAX_LINE 256

SOCKET nowSock;

DWORD WINAPI SendMessageThread(LPVOID param);

DWORD WINAPI ReceiveMessageThread(LPVOID param);

int main() {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    // added by wliu, for windows socket programming
    WSADATA WSAData;
    int WSAreturn;

    /* server address */
    struct sockaddr_in sin;
    // client address, added by wliu, for comments
    struct sockaddr_in remote;

    char buf[MAX_LINE];
    int len;
    SOCKET s, new_s;

    // added by wliu, for windows socket programming
    WSAreturn = WSAStartup(0x101, &WSAData);
    if (WSAreturn) {
        fprintf(stderr, "duplex-talk: WSA error.\n");
        exit(1);
    }

    /* build address data structure */
    // added by wliu, for string memory operation
    //bzero((char *)&sin, sizeof(sin));
    memset((char *) &sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("duplex-talk: socket failed.");
        exit(1);
    }
    if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
        perror("duplex-talk: bind failed.");
        exit(1);
    }

    // added by wliu, for comments
    fprintf(stderr, "server is ready in listening ...\n");

    listen(s, MAX_PENDING);

    HANDLE threadSend = CreateThread(NULL, 0, SendMessageThread,
                                     NULL, 0, NULL);

    /* wait for connection, then receive and print text */
    while (1) {
        // added by wliu, correction
        len = sizeof(struct sockaddr_in);

        if ((new_s = accept(s, (struct sockaddr *) &remote, &len)) < 0) {
            perror("duplex-talk: accept failed.");
            exit(1);
        }
        fprintf(stderr, "received a connection from %s : \n", inet_ntoa(remote.sin_addr));
        nowSock = new_s;

        HANDLE threadRecv = CreateThread(NULL, 0, ReceiveMessageThread,
                                         NULL, 0, NULL);
        WaitForSingleObject(threadRecv, INFINITE);
        CloseHandle(threadRecv);

        closesocket(nowSock);
    }

    TerminateThread(threadSend, 0);
    CloseHandle(threadSend);

    // added by wliu, for windows socket programming
    WSACleanup();
    return 1;
}

DWORD WINAPI SendMessageThread(LPVOID param) {
    char buf[MAX_LINE];
    int len;
    while (fgets(buf, sizeof(buf), stdin)) {
        buf[MAX_LINE - 1] = '\0';
        len = strlen(buf) + 1;
        if (send(nowSock, buf, len, 0) == SOCKET_ERROR) {
            fprintf(stderr, "send failed\n");
            continue;
        }
        fprintf(stderr, "send %d characters to client\n", len);
        printf("[server] %s\n", buf);
    }
    return 0;
}

DWORD WINAPI ReceiveMessageThread(LPVOID param) {
    char buf[MAX_LINE];
    int len;
    while ((len = recv(nowSock, buf, sizeof(buf), 0)) != SOCKET_ERROR && len != 0) {
        printf("[client] %s\n", buf);
    }
    return 0;
}
