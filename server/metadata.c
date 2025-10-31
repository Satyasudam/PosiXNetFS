// #include "metadata.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../common/utils.h"

void metadata_init();
void metadata_destroy();
static unsigned int hash(const char *s);
static pthread_mutex_t* get_or_create_mutex(const char* path);
void metadata_lock_file(const char* path);
void metadata_unlock_file(const char* path);
int metadata_check_version(const char* path,int client_version);

#define LOCK_TABLE_SIZE 1024
#define MAX_PATH_LEN 1024
typedef struct lock_entry {
    char path[MAX_PATH_LEN];
    pthread_mutex_t mutex;
    struct lock_entry *next;
} lock_entry_t;

static lock_entry_t* lock_table[LOCK_TABLE_SIZE];
static pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned int hash(const char *s) {
    unsigned long hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    return hash % LOCK_TABLE_SIZE;
}

void metadata_init() {
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        lock_table[i] = NULL;
    }
    LOG_INFO("Metadata lock table initialized.");
}

void metadata_destroy() {
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        lock_entry_t *entry = lock_table[i];
        while (entry) {
            lock_entry_t *temp = entry;
            entry = entry->next;
            pthread_mutex_destroy(&temp->mutex);
            free(temp);
        }
    }
    pthread_mutex_destroy(&table_mutex);
    LOG_INFO("Metadata lock table destroyed.");
}

static pthread_mutex_t* get_or_create_mutex(const char* path) {
    unsigned int index = hash(path);
    pthread_mutex_lock(&table_mutex);

    lock_entry_t *entry = lock_table[index];
    while (entry) {
        if (strcmp(entry->path, path) == 0) {
            pthread_mutex_unlock(&table_mutex);
            return &entry->mutex;
        }
        entry = entry->next;
    }

    lock_entry_t *new_entry = malloc(sizeof(lock_entry_t));
    strncpy(new_entry->path, path, MAX_PATH_LEN -1);
    new_entry->path[MAX_PATH_LEN - 1] = '\0';
    pthread_mutex_init(&new_entry->mutex, NULL);
    new_entry->next = lock_table[index];
    lock_table[index] = new_entry;

    pthread_mutex_unlock(&table_mutex);
    return &new_entry->mutex;
}

void metadata_lock_file(const char* path) {
    pthread_mutex_t* mutex = get_or_create_mutex(path);
    pthread_mutex_lock(mutex);
    LOG_DEBUG("Lock acquired for %s", path);
}

void metadata_unlock_file(const char* path) {
    pthread_mutex_t* mutex = get_or_create_mutex(path);
    pthread_mutex_unlock(mutex);
    LOG_DEBUG("Lock released for %s", path);
}

int metadata_check_version(const char* path, int client_version) {
    return 1;
}

