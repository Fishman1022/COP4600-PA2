#include "parser.h"
#include "threadlock.h" 
#include "chash.h"          // Added: contains struct defs
#include "thread_release.h" // Added: contains release wrapper
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

// --- Globals for Logging & Stats ---
FILE *logFile = NULL;
pthread_mutex_t logLock;
pthread_mutex_t statsLock;
long lockAcquisitions = 0;
long lockReleases = 0;

// --- Time Helper ---
long long main_timestamp() {  
  struct timeval te;  
  gettimeofday(&te, NULL); 
  long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; 
  return microseconds;  
}

// --- Logging Helper (For Commands) ---
void writeLog(int priority, const char *format, ...) {
    if (!logFile) return;
    
    long long ts = main_timestamp();
    va_list args;
    va_start(args, format);
    
    fprintf(logFile, "%lld: THREAD %d ", ts, priority);
    vfprintf(logFile, format, args);
    fprintf(logFile, "\n");
    fflush(logFile); 

    va_end(args);
}

// --- Stats Helpers ---
void incrementAcquisitions() {
    pthread_mutex_lock(&statsLock);
    lockAcquisitions++;
    pthread_mutex_unlock(&statsLock);
}

// Exposed via chash.h for thread_release.c
void incrementReleases() {
    pthread_mutex_lock(&statsLock);
    lockReleases++;
    pthread_mutex_unlock(&statsLock);
}

// --- Local Structs (Only needed here) ---
typedef struct {
    concurrentHashTable *table;
    Command cmd;
} ThreadArgs;

// --- Function Prototypes ---
concurrentHashTable* createHashTable(size_t num_buckets);
uint32_t jenkins_one_at_a_time_hash(char * name);
// threadReleaseWrapper is now in thread_release.h
void hashInsert(concurrentHashTable* table, char* name, int salary, int priority);
void hashDelete(concurrentHashTable* table, char* name, int priority);
void hashSearch(concurrentHashTable* table, char* name, int priority);
void updateSalary(concurrentHashTable* table, char* name, int salary, int priority);
void hashPrint(concurrentHashTable* table, int priority);
void logFinalStats(concurrentHashTable* table);

// --- Worker Thread Routine ---
void* thread_routine(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    usleep(50000); 

    switch(args->cmd.type) {
        case CMD_INSERT:
            hashInsert(args->table, args->cmd.name, args->cmd.salary, args->cmd.priority);
            break;
        case CMD_DELETE:
            hashDelete(args->table, args->cmd.name, args->cmd.priority);
            break;
        case CMD_UPDATE:
            updateSalary(args->table, args->cmd.name, args->cmd.salary, args->cmd.priority);
            break;
        case CMD_SEARCH:
            hashSearch(args->table, args->cmd.name, args->cmd.priority);
            break;
        case CMD_PRINT:
            hashPrint(args->table, args->cmd.priority);
            break;
        default:
            break;
    }

    free(args);
    return NULL;
}

int main(int argc, char *argv[]) {
    // 1. Setup Logging and Stats
    logFile = fopen("hash.log", "w");
    if (!logFile) {
        fprintf(stderr, "Error: Could not open hash.log for writing.\n");
        return 1;
    }
    pthread_mutex_init(&logLock, NULL);
    pthread_mutex_init(&statsLock, NULL);

    // 2. Create Table
    concurrentHashTable* table = createHashTable(100);
    
    const char *filename = "commands.txt";
    if (argc > 1) filename = argv[1];

    Command *cmds = NULL;
    size_t count = 0;
    if (parse_commands(filename, &cmds, &count) != 0) {
        fprintf(stderr, "Error parsing file.\n");
        fclose(logFile);
        return 1;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * count);
    
    for (size_t i = 0; i < count; i++) {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->table = table;
        args->cmd = cmds[i];
        pthread_create(&threads[i], NULL, thread_routine, args);
        usleep(50000); 
    }

    for (size_t i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }

    // 3. Write Final Stats
    logFinalStats(table);

    // Cleanup
    free(threads);
    free(cmds);
    
    if (logFile) fclose(logFile);
    pthread_mutex_destroy(&logLock);
    pthread_mutex_destroy(&statsLock);

    // --- PAUSE HERE (Silent) ---
    getchar();
    
    return 0;
}

// --- Implementation ---

concurrentHashTable* createHashTable(size_t num_buckets) {
    concurrentHashTable* table = (concurrentHashTable *)malloc(sizeof(concurrentHashTable));
    table->num_buckets = num_buckets;
    table->buckets = malloc(num_buckets * sizeof(hashBucket));
    for (int i = 0; i < num_buckets; i++) {
        table->buckets[i].head = NULL;
        threadLockInit(&table->buckets[i].rwlock); 
    }
    return table;
}

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

// --- COMMAND FUNCTIONS ---

void hashInsert(concurrentHashTable* table, char* name, int salary, int priority) {
    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    writeLog(priority, "INSERT,%u,%s,%d", hashVal, name, salary);
    
    threadWriteLock(&table->buckets[index].rwlock, priority, logFile);
    incrementAcquisitions(); 
    
    hashRecord* current = table->buckets[index].head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
             printf("Insert failed. Entry %u is a duplicate.\n", hashVal);
             threadReleaseWrapper(table, hashVal, priority, 1); 
             return;
        }
        current = current->next;
    }

    hashRecord* rec = malloc(sizeof(hashRecord));
    strcpy(rec->name, name);
    rec->salary = salary;
    rec->hash = hashVal;
    rec->next = table->buckets[index].head;
    table->buckets[index].head = rec;

    printf("Inserted %u,%s,%d\n", hashVal, name, salary);
    
    threadReleaseWrapper(table, hashVal, priority, 1);
}

void hashDelete(concurrentHashTable* table, char* name, int priority) {
    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    writeLog(priority, "DELETE,%u,%s", hashVal, name);

    threadWriteLock(&table->buckets[index].rwlock, priority, logFile);
    incrementAcquisitions(); 
    
    hashRecord* current = table->buckets[index].head;
    hashRecord* prev = NULL;
    int found = 0;
    uint32_t oldSalary = 0;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            found = 1;
            oldSalary = current->salary;
            if (prev == NULL) table->buckets[index].head = current->next;
            else prev->next = current->next;
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    if (found) {
        printf("Deleted record for %u,%s,%u\n", hashVal, name, oldSalary);
    } else {
        printf("%s not found.\n", name); 
    }

    threadReleaseWrapper(table, hashVal, priority, 1);
}

void updateSalary(concurrentHashTable* table, char* name, int salary, int priority) {
    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    writeLog(priority, "UPDATE,%u,%s,%d", hashVal, name, salary);

    threadWriteLock(&table->buckets[index].rwlock, priority, logFile);
    incrementAcquisitions(); 
    
    hashRecord* current = table->buckets[index].head;
    int found = 0;
    uint32_t oldSalary = 0;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            found = 1;
            oldSalary = current->salary;
            current->salary = salary;
            break;
        }
        current = current->next;
    }

    if (found) {
        printf("Updated record %u from %u,%s,%u to %u,%s,%u\n", 
               hashVal, hashVal, name, oldSalary, hashVal, name, salary);
    } else {
        printf("Update failed. Entry %u not found.\n", hashVal);
    }

    threadReleaseWrapper(table, hashVal, priority, 1);
}

void hashSearch(concurrentHashTable* table, char* name, int priority) {
    uint32_t hashVal = jenkins_one_at_a_time_hash(name);
    size_t index = hashVal % table->num_buckets;

    writeLog(priority, "SEARCH,%u,%s", hashVal, name);

    threadReadLock(&table->buckets[index].rwlock, priority, logFile);
    incrementAcquisitions(); 
    
    hashRecord* curr = table->buckets[index].head;
    int found = 0;

    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) {
            printf("Found: %u,%s,%u\n", curr->hash, curr->name, curr->salary);
            found = 1;
            break; 
        }
        curr = curr->next;
    }

    if (!found) {
        printf("%s not found.\n", name);
    }

    threadReleaseWrapper(table, hashVal, priority, 0); 
}

int compareRecords(const void *a, const void *b) {
    hashRecord *recA = *(hashRecord **)a;
    hashRecord *recB = *(hashRecord **)b;
    if (recA->hash < recB->hash) return -1;
    if (recA->hash > recB->hash) return 1;
    return 0;
}

void hashPrint(concurrentHashTable* table, int priority) {
    writeLog(priority, "PRINT");
    
    incrementAcquisitions();
    // Manual log for the "Logical" lock event
    long long ts = main_timestamp();
    fprintf(logFile, "%lld: THREAD %d READ LOCK ACQUIRED\n", ts, priority);

    int capacity = 100;
    int count = 0;
    hashRecord **snapshot = malloc(sizeof(hashRecord*) * capacity);

    for (size_t i = 0; i < table->num_buckets; i++) {
        pthread_rwlock_rdlock(&table->buckets[i].rwlock.rwlock);

        hashRecord *curr = table->buckets[i].head;
        while (curr != NULL) {
            if (count >= capacity) {
                capacity *= 2;
                snapshot = realloc(snapshot, sizeof(hashRecord*) * capacity);
            }
            hashRecord *copy = malloc(sizeof(hashRecord));
            memcpy(copy, curr, sizeof(hashRecord));
            copy->next = NULL;
            snapshot[count++] = copy;
            curr = curr->next;
        }
        
        pthread_rwlock_unlock(&table->buckets[i].rwlock.rwlock);
    }

    incrementReleases();
    ts = main_timestamp();
    fprintf(logFile, "%lld: THREAD %d READ LOCK RELEASED\n", ts, priority);

    qsort(snapshot, count, sizeof(hashRecord*), compareRecords);

    printf("Current Database:\n");
    for (int i = 0; i < count; i++) {
        printf("%u,%s,%u\n", snapshot[i]->hash, snapshot[i]->name, snapshot[i]->salary);
        free(snapshot[i]); 
    }

    free(snapshot);
}

void logFinalStats(concurrentHashTable* table) {
    if (!logFile) return;

    fprintf(logFile, "\nNumber of lock acquisitions: %ld\n", lockAcquisitions);
    fprintf(logFile, "Number of lock releases: %ld\n", lockReleases);
    fprintf(logFile, "Final Table:\n");

    int capacity = 100;
    int count = 0;
    hashRecord **snapshot = malloc(sizeof(hashRecord*) * capacity);

    for (size_t i = 0; i < table->num_buckets; i++) {
        hashRecord *curr = table->buckets[i].head;
        while (curr != NULL) {
            if (count >= capacity) {
                capacity *= 2;
                snapshot = realloc(snapshot, sizeof(hashRecord*) * capacity);
            }
            snapshot[count++] = curr; 
            curr = curr->next;
        }
    }

    qsort(snapshot, count, sizeof(hashRecord*), compareRecords);

    for (int i = 0; i < count; i++) {
        fprintf(logFile, "%u,%s,%u\n", snapshot[i]->hash, snapshot[i]->name, snapshot[i]->salary);
    }

    free(snapshot);
}