#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Network bandwidth measurement client
// Connects to server and measures upstream (client->server) and downstream (server->client) throughput

#define PORT 2444               // Server listening port
#define BUFFER_SIZE 65536       // 64KB buffer size for sending/receiving data
#define period_sec 8.0          // Test duration in seconds

// Get current time in seconds using CPU clock
double now_sec() {
    return (double)clock() / CLOCKS_PER_SEC;
}

int main() {
    // Initialize Windows Socket library
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    // Create client socket (IPv4, TCP)
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Configure server address structure
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;              // IPv4
    addr.sin_port = htons(PORT);            // Convert port to network byte order

    // Use inet_addr to avoid old MinGW issues
    // Connect to localhost (127.0.0.1)
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    printf("[Client] Connected to server.\n");

    // Allocate buffer for data transfer
    char *buffer = malloc(BUFFER_SIZE);
    memset(buffer, 'C', BUFFER_SIZE);   // Fill buffer with test data

    /* -------------------------
       UPSTREAM TEST (Client → Server)
       Send data to server and measure throughput
       ------------------------- */
    long long total_sent = 0;           // Total bytes sent
    long long interval_sent = 0;        // Bytes sent in current interval
    double start = now_sec();
    int n;                              // Bytes sent in last send() call
    double last_print = start;          // Time of last progress print

    printf("\nUPSTREAM TEST (Client -> Server)\n");
    printf("Interval     Transfer     Bitrate\n");

    // Send data for specified duration
    while (now_sec() - start < period_sec) {
        // send() sends data to server
        // Returns bytes sent, -1 on error
        n = send(sockfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;              // Stop if send failed or connection closed
        total_sent += n;
        interval_sent += n;
        
        // Print speed every 1 second
        double now = now_sec();
        if (now - last_print >= 1.0) {
            double interval_MBps = (interval_sent / 1024.0 / 1024.0);
            double interval_Mbps = interval_MBps * 8;  // Convert bytes/s to bits/s
            printf("%.1f-%.1f sec  %.2f MB   %.2f Mbps\n",
                   now - start - 1.0, now - start, interval_MBps, interval_Mbps);
            interval_sent = 0;
            last_print = now;
        }
    }

    // Calculate and print upstream results
    double actual_duration = now_sec() - start;
    double upstream_MBps = (total_sent / 1024.0 / 1024.0) / actual_duration;
    printf("%.1f-%.1f sec  %.2f MB   %.2f Mbps (total)\n",
           0.0, actual_duration, total_sent / 1024.0 / 1024.0, upstream_MBps * 8);

    /* -------------------------
       DOWNSTREAM TEST (Server → Client)
       Receive data from server and measure throughput
       ------------------------- */
    long long total_recv = 0;           // Total bytes received
    long long interval_recv = 0;        // Bytes received in current interval
    start = now_sec();
    last_print = start;

    printf("\nDOWNSTREAM TEST (Server -> Client)\n");
    printf("Interval     Transfer     Bitrate\n");
    
    // Receive data for specified duration
    while (now_sec() - start < period_sec) {
        // recv() receives up to BUFFER_SIZE bytes from server
        // Returns bytes received, 0 if connection closed, -1 on error
        n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break;              // Stop if no data or connection closed
        total_recv += n;
        interval_recv += n;
        
        // Print speed every 1 second
        double now = now_sec();
        if (now - last_print >= 1.0) {
            double interval_MBps = (interval_recv / 1024.0 / 1024.0);
            double interval_Mbps = interval_MBps * 8;  // Convert bytes/s to bits/s
            printf("%.1f-%.1f sec  %.2f MB   %.2f Mbps\n",
                   now - start - 1.0, now - start, interval_MBps, interval_Mbps);
            interval_recv = 0;
            last_print = now;
        }
    }

    // Calculate and print downstream results
    actual_duration = now_sec() - start;
    double downstream_MBps = (total_recv / 1024.0 / 1024.0) / actual_duration;
    printf("%.1f-%.1f sec  %.2f MB   %.2f Mbps (total)\n",
           0.0, actual_duration, total_recv / 1024.0 / 1024.0, downstream_MBps * 8);

    // Print summary
    printf("\n-------- Test Results --------\n");
    printf("[Client] Upstream:   %.2f Mbps\n", upstream_MBps * 8);
    printf("[Client] Downstream: %.2f Mbps\n", downstream_MBps * 8);
    printf("[Client] Average:    %.2f Mbps\n", (upstream_MBps + downstream_MBps) * 4);
    printf("------------------------------\n");
    
    // Cleanup: release allocated memory and close socket
    free(buffer);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
