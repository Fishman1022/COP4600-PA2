#ifndef THREADLOCK_H
#define THREADLOCK_H
#include <pthread.h>
#include <stdio.h>

typedef struct {
    pthread_rwlock_t rwlock;
} ThreadRWLock;

void threadLockInit(ThreadRWLock *lk);
void threadLockDestroy(ThreadRWLock *lk);
void threadReadLock(ThreadRWLock *lk, int priority, FILE *log);
void threadReadUnlock(ThreadRWLock *lk, int priority, FILE *log);
void threadWriteLock(ThreadRWLock *lk, int priority, FILE *log);
void threadWriteUnlock(ThreadRWLock *lk, int priority, FILE *log);

#endif
