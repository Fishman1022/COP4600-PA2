#include "thread_release.h"
#include "threadlock.h"

void threadReleaseWrapper(concurrentHashTable* table, uint32_t hash, int priority, int isWriter) {
    if (!table) return;
    size_t index = hash % table->num_buckets;
    
    // Perform the unlock using the threadlock helper
    if (isWriter) {
        threadWriteUnlock(&table->buckets[index].rwlock, priority, logFile);
    } else {
        threadReadUnlock(&table->buckets[index].rwlock, priority, logFile);
    }

    // Call the shared stats incrementer defined in chash.c
    incrementReleases();
}