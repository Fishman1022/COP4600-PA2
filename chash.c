#include "parser.h"
#include "threadlock.h"
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); 
    long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; 
    return microseconds; 
}

typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

// Organizes Hash records into an array, lets us track if one is locked/released
typedef struct {
    hashRecord *head;
    pthread_rwlock_t rwlock;
} hashBucket;

// Hash Table Wrapper
typedef struct {
    hashBucket *buckets;
    size_t num_buckets;
} concurrentHashTable;

concurrentHashTable* createHashTable(size_t num_buckets) {
    // Creates table, then initializes num buckets and creates array structure for each bucket
    concurrentHashTable* table = (concurrentHashTable *)malloc(sizeof(concurrentHashTable));
    if (!table) return NULL;
    
    table->num_buckets = num_buckets;
    table->buckets = malloc(num_buckets * sizeof(hashBucket));
    if (!table->buckets) {
        free(table);
        return NULL;
    }

    // Initializes each bucket
    for (int i = 0; i < num_buckets; i++) {
        table->buckets[i].head = NULL; // Fixed: accessing array index i
        pthread_rwlock_init(&table->buckets[i].rwlock, NULL);
    }

    return table;
}

// Jenkins one at a time hash
uint32_t jenkins_one_at_a_time_hash(char * name) { 
    size_t i = 0;
    uint32_t hash = 0;
    while (i != strlen(name)) {
        hash += name[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

// Prototypes
void readCommandsFile();
void threadRelease(concurrentHashTable* table, uint32_t hash, int isWriter, int priority);
void threadLock(concurrentHashTable* table);
void hashInsert(concurrentHashTable* table, char* name, int salary, int priority);
void hashDelete(concurrentHashTable* table, char* name, int priority);
void hashSearch(concurrentHashTable* table, char* name, int priority);
void hashPrint(concurrentHashTable* table);
void writeLogFile();

int main() {
    concurrentHashTable* table = createHashTable(100);

    // Populate data
    hashInsert(table, "Broc", 50000, 1);
    hashInsert(table, "Alice", 60000, 2);
    
    // Demonstrate Search
    hashSearch(table, "Broc", 1);
    hashSearch(table, "Unknown", 3);

    // Demonstrate Print
    hashPrint(table);

    // Clean up
    hashDelete(table, "Broc", 1);

    return 0;
}

void threadRelease(concurrentHashTable* table, uint32_t hash, int isWriter, int priority) {
    if (!table) return;

    size_t index = hash % table->num_buckets;

    pthread_rwlock_unlock(&table->buckets[index].rwlock);

    long long timestamp = current_timestamp();
    if(isWriter) {
        printf("%lld,THREAD %d WRITE LOCK RELEASED\n", timestamp, priority);
    } else {
        printf("%lld,THREAD %d READ LOCK RELEASED\n", timestamp, priority);
    }
}

void hashInsert(concurrentHashTable* table, char* name, int salary, int priority) {
    if (!table) return;

    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    // Use threadLock equivalent or manual lock? Manual logic provided in prompt:
    pthread_rwlock_wrlock(&table->buckets[index].rwlock);

    hashRecord* current = table->buckets[index].head;
    while (current != NULL) {
        if (current->hash == hashVal) { // Note: Should ideally check name with strcmp for hash collisions
            if (strcmp(current->name, name) == 0) {
                 printf("Insert failed. Entry %u is a duplicate.\n", hashVal);
                 threadRelease(table, hashVal, 1, priority); // Use helper to unlock
                 return;
            }
        }
        current = current->next;
    }

    hashRecord* rec = (hashRecord*) malloc(sizeof(hashRecord));
    strcpy(rec->name, name);
    rec->salary = salary;
    rec->hash = hashVal;

    rec->next = table->buckets[index].head;
    table->buckets[index].head = rec;

    printf("Inserted %u,%s,%d\n", hashVal, name, salary);
    
    threadRelease(table, hashVal, 1, priority);
}

void hashDelete(concurrentHashTable* table, char* name, int priority) {
    if (!table) return;

    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    pthread_rwlock_wrlock(&table->buckets[index].rwlock);

    hashRecord* current = table->buckets[index].head;
    hashRecord* prev = NULL;
    int found = 0;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            found = 1;
            if (prev == NULL) {
                table->buckets[index].head = current->next;
            } else {
                prev->next = current->next;
            }
            printf("Deleted record for %u,%s,%u\n", hashVal, name, current->salary);
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    // Manual unlock to match prompt logic, or use threadRelease
    pthread_rwlock_unlock(&table->buckets[index].rwlock);
    
    // Logging adapted from threadRelease style
    long long timestamp = current_timestamp();
    printf("%lld,THREAD %d WRITE LOCK RELEASED\n", timestamp, priority);

    if (!found) {
        printf("Entry %u not deleted. Not in database.\n", hashVal);
    }
}

// --- Imported and Adapted Functions ---

void hashSearch(concurrentHashTable* table, char* name, int priority) {
    if (!table) return;

    long long ts = current_timestamp();
    printf("%lld,THREAD %d WAITING FOR MY TURN\n", ts, priority);

    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    pthread_rwlock_rdlock(&table->buckets[index].rwlock);
    
    ts = current_timestamp();
    printf("%lld,THREAD %d READ LOCK ACQUIRED\n", ts, priority);

    hashRecord* curr = table->buckets[index].head;
    int found = 0;

    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) {
            printf("SEARCH Found: %u,%s,%u\n", curr->hash, curr->name, curr->salary);
            found = 1;
            break; 
        }
        curr = curr->next;
    }

    if (!found) {
        printf("SEARCH No Record Found for %s\n", name);
    }

    // Unlock and Log
    threadRelease(table, hashVal, 0, priority); // 0 for Read Lock
}

void hashPrint(concurrentHashTable* table) {
    if (!table) return;

    // No priority passed to hashPrint in this prototype, so we skip standard priority logging 
    // or use a default if needed. Standard print usually dumps the whole DB.
    
    FILE *outfile = fopen("output.txt", "a");
    if (!outfile) {
        printf("Error opening output.txt\n");
        return;
    }

    printf("PRINT Current Database:\n");
    fprintf(outfile, "Current Database:\n");

    // Iterate over ALL buckets
    for (size_t i = 0; i < table->num_buckets; i++) {
        // Lock individual bucket
        pthread_rwlock_rdlock(&table->buckets[i].rwlock);
        
        hashRecord *curr = table->buckets[i].head;
        while (curr != NULL) {
            printf("%u,%s,%u\n", curr->hash, curr->name, curr->salary);
            fprintf(outfile, "%u,%s,%u\n", curr->hash, curr->name, curr->salary);
            curr = curr->next;
        }

        pthread_rwlock_unlock(&table->buckets[i].rwlock);
    }

    fclose(outfile);
}