#include "threadlock.h"
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

static long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    return (long long)te.tv_sec * 1000000LL + te.tv_usec;
}

void threadLockInit(ThreadRWLock *lk) {
    pthread_rwlock_init(&lk->rwlock, NULL);
}

void threadLockDestroy(ThreadRWLock *lk) {
    pthread_rwlock_destroy(&lk->rwlock);
}

void threadReadLock(ThreadRWLock *lk, int priority, FILE *log) {
    fprintf(log, "%lld: THREAD %d WAITING FOR MY TURN\n", current_timestamp(), priority);
    pthread_rwlock_rdlock(&lk->rwlock);
    fprintf(log, "%lld: THREAD %d AWAKENED FOR WORK\n", current_timestamp(), priority);
    fprintf(log, "%lld: THREAD %d READ LOCK ACQUIRED\n", current_timestamp(), priority);
}

void threadReadUnlock(ThreadRWLock *lk, int priority, FILE *log) {
    fprintf(log, "%lld: THREAD %d READ LOCK RELEASED\n", current_timestamp(), priority);
    pthread_rwlock_unlock(&lk->rwlock);
}

void threadWriteLock(ThreadRWLock *lk, int priority, FILE *log) {
    fprintf(log, "%lld: THREAD %d WAITING FOR MY TURN\n", current_timestamp(), priority);
    pthread_rwlock_wrlock(&lk->rwlock);
    fprintf(log, "%lld: THREAD %d AWAKENED FOR WORK\n", current_timestamp(), priority);
    fprintf(log, "%lld: THREAD %d WRITE LOCK ACQUIRED\n", current_timestamp(), priority);
}

void threadWriteUnlock(ThreadRWLock *lk, int priority, FILE *log) {
    fprintf(log, "%lld: THREAD %d WRITE LOCK RELEASED\n", current_timestamp(), priority);
    pthread_rwlock_unlock(&lk->rwlock);
}
