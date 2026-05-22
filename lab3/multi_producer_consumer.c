#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define ITEMS_PER_PRODUCER 10

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0, out = 0;

int total_items = 0;
int produced_count = 0;
int consumed_count = 0;

int N, M;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        pthread_mutex_lock(&lock);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &lock);
        }

        int item = id * 100 + i;  // unique item per producer

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        produced_count++;

        printf("Producer %d produced %d (count=%d)\n", id, item, count);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);

        usleep(50000);
    }

    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&lock);

        while (count == 0 && consumed_count < total_items) {
            pthread_cond_wait(&not_empty, &lock);
        }

        // Exit condition: all items consumed
        if (consumed_count >= total_items) {
            pthread_cond_broadcast(&not_empty);
            pthread_mutex_unlock(&lock);
            break;
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        consumed_count++;

        printf("Consumer %d consumed %d (count=%d)\n", id, item, count);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);

        usleep(80000);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <num_producers> <num_consumers>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);

    total_items = N * ITEMS_PER_PRODUCER;

    pthread_t producers[N], consumers[M];
    int pid[N], cid[M];

    // create producers
    for (int i = 0; i < N; i++) {
        pid[i] = i;
        pthread_create(&producers[i], NULL, producer, &pid[i]);
    }

    // create consumers
    for (int i = 0; i < M; i++) {
        cid[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &cid[i]);
    }

    // join producers
    for (int i = 0; i < N; i++) {
        pthread_join(producers[i], NULL);
    }

    // wake up consumers so they can exit
    pthread_mutex_lock(&lock);
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&lock);

    // join consumers
    for (int i = 0; i < M; i++) {
        pthread_join(consumers[i], NULL);
    }

    printf("All done. Produced=%d Consumed=%d\n", produced_count, consumed_count);

    return 0;
}
