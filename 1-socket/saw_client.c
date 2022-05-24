// added by wliu, for windows socket programming
#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <time.h>

#define SERVER_PORT 5432
#define MAX_LINE 256

static const int NUM_CASES = 4;

/*
 * 0 do not send
 * 1 send
 */
static const int CASES[4][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {0, 1, 1},
        {1, 1, 1}
};

static const int TIMEOUT = 5000;

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
    char buf[MAX_LINE];
    SOCKET s;

    int caseID;
    if (argc == 3) {
        host = argv[1];
        caseID = atoi(argv[2]);
    } else {
        fprintf(stderr, "usage: saw_client host case\n");
        exit(1);
    }

    // added by wliu, for windows socket programming
    WSAreturn = WSAStartup(0x101, &WSAData);
    if (WSAreturn) {
        fprintf(stderr, "saw_client: WSA error.\n");
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "saw_client: unknown host: %s\n", host);
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
        perror("saw_client: socket failed.");
        exit(1);
    }
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        perror("saw_client: connect failed.");
        closesocket(s);
        exit(1);
    }

    fprintf(stderr, "client is connecting to %s\n", host);

    for (int i = 0; i < 3; i++) {
        int *pID = (int *) buf;
        *pID = i;
        printf("send packet %d to server\n", i);
        if (CASES[caseID][i] == 0) {
            fprintf(stderr, "request packet dropped\n");
        } else if (CASES[caseID][i] == 1) {
            printf("request packet %d is sent\n", i);
            send(s, buf, sizeof(int), 0);
        }

        clock_t startTime = clock();
        while (1) {
            int waitTime = (clock() - startTime) * 1000 / CLOCKS_PER_SEC;
            if (waitTime > TIMEOUT) {
                fprintf(stderr, "ack timeout for %d, is it missing?\n", i);
                break;
            }
            unsigned long len;
            ioctlsocket(s, FIONREAD, &len);
            if (len == 0) continue;
            recv(s, buf, sizeof(buf), 0);
            if (*pID != i) {
                fprintf(stderr, "received an invalid ack for %d\n", *pID);
                continue;
            }
            printf("ack received for %d\n", *pID);
            break;
        }
    }
    Sleep(1000);
    closesocket(s);
    printf("socket closed\n");

    // added by wliu, for windows socket programming
    WSACleanup();
    return 0;
}

