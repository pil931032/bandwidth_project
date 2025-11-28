#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 65536   // 64KB
#define period_sec 3.0

double now_sec() {
    return (double)clock() / CLOCKS_PER_SEC;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    // Use inet_addr to avoid old MinGW issues
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    printf("[Client] Connected to server.\n");

    char *buffer = malloc(BUFFER_SIZE);
    memset(buffer, 'C', BUFFER_SIZE);

    /* -------------------------
       UPSTREAM (Client → Server)
       ------------------------- */
    long long total_sent = 0;
    double start = now_sec();
    int n;

    while (now_sec() - start < period_sec) {
        n = send(sockfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;
        total_sent += n;
    }

    double upstream_MBps = (total_sent / 1024.0 / 1024.0) / period_sec;
    printf("[Client] Upstream: %.2f MB/s (%.2f Mbps)\n",
           upstream_MBps,
           upstream_MBps * 8);

    /* -------------------------
       DOWNSTREAM (Server → Client)
       ------------------------- */
    long long total_recv = 0;
    start = now_sec();

    while (now_sec() - start < period_sec) {
        n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;
        total_recv += n;
    }

    double downstream_MBps = (total_recv / 1024.0 / 1024.0) / period_sec;
    printf("[Client] Downstream: %.2f MB/s (%.2f Mbps)\n",
           downstream_MBps,
           downstream_MBps * 8);

    /* cleanup */
    free(buffer);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
