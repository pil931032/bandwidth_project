//compile with:
//gcc server.c -o server.exe -lws2_32
//gcc client.c -o client.exe -lws2_32
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

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);
    printf("[Server] Waiting for connection...\n");

    SOCKET client = accept(server_fd, NULL, NULL);
    printf("[Server] Client connected.\n");

    char *buffer = malloc(BUFFER_SIZE);

    /* -------------------------
       UPSTREAM TEST (Client → Server)
       ------------------------- */
    double start = now_sec();
    long long total_recv = 0;
    int n;

    while (now_sec() - start < period_sec) {
        n = recv(client, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;
        total_recv += n;
    }

    double upstream_MBps = (total_recv / 1024.0 / 1024.0) / period_sec;
    printf("[Server] Upstream: %.2f MB/s (%.2f Mbps)\n",
           upstream_MBps,
           upstream_MBps * 8);

    /* -------------------------
       DOWNSTREAM TEST (Server → Client)
       ------------------------- */
    memset(buffer, 'S', BUFFER_SIZE);
    long long total_sent = 0;
    start = now_sec();

    while (now_sec() - start < period_sec) {
        n = send(client, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;
        total_sent += n;
    }

    double downstream_MBps = (total_sent / 1024.0 / 1024.0) / period_sec;
    printf("[Server] Downstream: %.2f MB/s (%.2f Mbps)\n",
           downstream_MBps,
           downstream_MBps * 8);

    /* cleanup */
    free(buffer);
    closesocket(client);
    closesocket(server_fd);
    WSACleanup();

    return 0;
}
