// added by wliu, for windows socket programming
#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>
#include <string.h>
#include <conio.h>

#define SERVER_PORT 5432
#define MAX_LINE 256

DWORD WINAPI SendMessageThread(LPVOID param);

DWORD WINAPI ReceiveMessageThread(LPVOID param);

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    // added by wliu, for windows socket programming
    WSADATA WSAData;
    int WSAreturn;

    //FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    SOCKET s;

    if (argc == 2) {
        host = argv[1];
    } else {
        fprintf(stderr, "usage: duplex-talk host\n");
        exit(1);
    }

    // added by wliu, for windows socket programming
    WSAreturn = WSAStartup(0x101, &WSAData);
    if (WSAreturn) {
        fprintf(stderr, "duplex-talk: WSA error.\n");
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "duplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    // modified by wliu, for string memory operation
    //bzero((char *)&sin, sizeof(sin));
    //bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    memset((char *) &sin, 0, sizeof(sin));
    memcpy((char *) &sin.sin_addr, hp->h_addr, hp->h_length);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);

    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("duplex-talk: socket failed.");
        exit(1);
    }
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        perror("duplex-talk: connect failed.");
        closesocket(s);
        exit(1);
    }

    fprintf(stderr, "client is connecting to %s\n", host);

    HANDLE threads[2];
    DWORD threadIDs[2];
    threads[0] = CreateThread(NULL,
                              0,
                              SendMessageThread,
                              &s,
                              0,
                              &threadIDs[0]);
    threads[1] = CreateThread(NULL,
                              0,
                              ReceiveMessageThread,
                              &s,
                              0,
                              &threadIDs[1]);
    WaitForSingleObject(threads[1], INFINITE);
    TerminateThread(threads[0], 0);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);

    // added by wliu, for windows socket programming
    WSACleanup();
    return 1;
}

DWORD WINAPI SendMessageThread(LPVOID param) {
    char buf[MAX_LINE];
    int len;
    SOCKET s = *((SOCKET *) param);
    while (fgets(buf, sizeof(buf), stdin)) {
        buf[MAX_LINE - 1] = '\0';
        len = strlen(buf) + 1;
        send(s, buf, len, 0);
        fprintf(stderr, "send %d characters to server\n", len);
        printf("[client] %s\n", buf);
    }
    return 0;
}

DWORD WINAPI ReceiveMessageThread(LPVOID param) {
    char buf[MAX_LINE];
    int len;
    SOCKET s = *((SOCKET *) param);
    while ((len = recv(s, buf, sizeof(buf), 0)) != SOCKET_ERROR && len != 0) {
        printf("[server] %s\n", buf);
    }
    return 0;
}
