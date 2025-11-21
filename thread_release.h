#ifndef THREAD_RELEASE_H
#define THREAD_RELEASE_H

#include "chash.h"

void threadReleaseWrapper(concurrentHashTable* table, uint32_t hash, int priority, int isWriter);

#endif