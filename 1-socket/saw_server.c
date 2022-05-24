// added by wliu, for windows socket programming
#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>
#include <string.h>

#define SERVER_PORT 5432
#define MAX_PENDING 5
#define MAX_LINE 256

static const int NUM_CASES = 4;

/*
 * 0 do not ack
 * 1 ack
 * 2 delayed ack
 */
static const int CASES[4][3] = {
        {1, 1, 1},
        {0, 1, 1},
        {1, 1, 1},
        {2, 2, 1}
};

struct ThreadParams {
    SOCKET s;
    int pID;
};

int caseID;

DWORD WINAPI ServerThread(LPVOID param);

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    // added by wliu, for windows socket programming
    WSADATA WSAData;
    int WSAreturn;

    if (argc < 2) {
        fprintf(stderr, "usage: saw_server case\n");
        exit(1);
    }
    caseID = atoi(argv[1]);

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
        fprintf(stderr, "saw_server: WSA error.\n");
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
        perror("saw_server: socket failed.");
        exit(1);
    }
    if ((bind(s, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
        perror("saw_server: bind failed.");
        exit(1);
    }

    // added by wliu, for comments
    printf("server is ready in listening ...\n");

    listen(s, MAX_PENDING);

    /* wait for connection, then receive and print text */
    while (1) {
        // added by wliu, correction
        len = sizeof(struct sockaddr_in);

        if ((new_s = accept(s, (struct sockaddr *) &remote, &len)) < 0) {
            perror("saw_server: accept failed.");
            exit(1);
        }
        printf("received a connection from %s : \n", inet_ntoa(remote.sin_addr));

        DWORD threadID;
        while ((len = recv(new_s, buf, sizeof(buf), 0)) != SOCKET_ERROR) {
            if (len == 0) break;
            struct ThreadParams *params = malloc(sizeof(struct ThreadParams));
            params->s = new_s;
            params->pID = *((int *) buf);
            CreateThread(NULL, 0, ServerThread,
                         params, 0, &threadID);
        }

        Sleep(1000);
        closesocket(new_s);
        printf("socket closed\n");
    }

    // added by wliu, for windows socket programming
    WSACleanup();
    return 0;
}

DWORD WINAPI ServerThread(LPVOID param) {
    char buf[MAX_LINE];
    struct ThreadParams *params = (struct ThreadParams *) param;
    SOCKET s = params->s;
    int pID = params->pID;
    free(param);

    *((int *) buf) = pID;
    printf("received packet %d from client\n", pID);
    Sleep(1000);
    if (CASES[caseID][pID] == 0) {
        fprintf(stderr, "ack packet dropped\n");
    } else if (CASES[caseID][pID] == 1) {
        printf("ack packet %d is sent\n", pID);
        send(s, buf, sizeof(int), 0);
    } else if (CASES[caseID][pID] == 2) {
        fprintf(stderr, "ack %d delayed for 4s\n", pID);
        Sleep(4000);
        printf("ack packet %d is sent\n", pID);
        send(s, buf, sizeof(int), 0);
    }

    return 0;
}
