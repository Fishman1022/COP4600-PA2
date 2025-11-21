#ifndef CHASH_H
#define CHASH_H

#include <stdint.h>
#include <stdio.h>
#include "threadlock.h"

// --- Shared Struct Definitions ---

typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

typedef struct {
    hashRecord *head;
    ThreadRWLock rwlock; 
} hashBucket;

typedef struct {
    hashBucket *buckets;
    size_t num_buckets;
} concurrentHashTable;

// --- Shared Globals/Functions ---
// These are defined in chash.c but needed by thread_release.c
extern FILE *logFile;
void incrementReleases();

#endif