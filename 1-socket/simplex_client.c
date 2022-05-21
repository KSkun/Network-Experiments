// added by wliu, for windows socket programming
#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>
#include <string.h>
#include <conio.h>

#define SERVER_PORT 5432
#define MAX_LINE 256

int main(int argc, char *argv[]) {
    // added by wliu, for windows socket programming
    WSADATA WSAData;
    int WSAreturn;

    //FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE];
    SOCKET s;
    int len;

    if (argc == 2) {
        host = argv[1];
    } else {
        fprintf(stderr, "usage: simplex-talk host\n");
        exit(1);
    }

    // added by wliu, for windows socket programming
    WSAreturn = WSAStartup(0x101, &WSAData);
    if (WSAreturn) {
        fprintf(stderr, "simplex-talk: WSA error.\n");
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
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
        perror("simplex-talk: socket failed.");
        exit(1);
    }
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect failed.");
        closesocket(s);
        exit(1);
    }

    fprintf(stderr, "client is connecting to %s\n", host);

    /* main loop: get and send lines of text */
    while (fgets(buf, sizeof(buf), stdin)) {
        buf[MAX_LINE - 1] = '\0';
        len = strlen(buf) + 1;
        send(s, buf, len, 0);
    }

    // added by wliu, for windows socket programming
    WSACleanup();
    return 1;
}

