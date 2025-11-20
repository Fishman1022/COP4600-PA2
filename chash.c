#include "parser.h"
#include "threadlock.h"
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include "sys/time.h"

long long current_timestamp() {
  struct timeval te;
  gettimeofday(&te, NULL); // get current time  
  long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // calculate milliseconds  
  return microseconds;  
}

typedef struct hash_struct {
  uint32_t hash;
  char name[50];
  uint32_t salary;
  struct hash_struct *next;
} hashRecord;

//Organizes Hash records into an array, lets us track if one is locked/released
typedef struct {
  hashRecord *head;
  pthread_rwlock_t rwlock;
} hashBucket;

//Hash Table Wrapper
typedef struct {
  hashBucket *buckets;
  size_t num_buckets;
} concurrentHashTable;

concurrentHashTable* createHashTable(size_t num_buckets) {
  //Creates table, then initializes num buckets and creates array structure for each bucket
  concurrentHashTable* table = (concurrentHashTable *)malloc(sizeof(concurrentHashTable));
  table->num_buckets = num_buckets;
  table->buckets = malloc(num_buckets * sizeof(hashBucket));

  //Initializes each bucket
  for (int i = 0; i < num_buckets; i++) {
    table->buckets->head = NULL;
    pthread_rwlock_init(&table->buckets[i].rwlock, NULL);
  }

  return table;
}

// Jenkins one at a time hash (Copied from https://en.wikipedia.org/wiki/Jenkins_hash_function)
// per assignment details.
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

//These are template, you can change them to whateverr yoou think wroks best lol
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

  hashInsert(table, "Broc", 0, 1);
  hashDelete(table, "Broc", 1);

  return 0;
}

void threadRelease(concurrentHashTable* table, uint32_t hash, int isWriter, int priority) {
  if (!table) return;

  size_t index = hash %table->num_buckets;

  pthread_rwlock_unlock(&table->buckets[index].rwlock);

  long long timestamp = current_timestamp();
  if(isWriter) {
    printf("%lld, THREAD %d WRITE LOCK RELEASED\n", timestamp, priority);
  } else {
    printf("%lld, THREAD %d READ LOCK RELEASED\n", timestamp, priority);
  }
}

void hashInsert(concurrentHashTable* table, char* name, int salary, int priority) {
  if (!table) return;

  uint32_t hashVal = jenkins_one_at_a_time_hash(name);
  size_t index = hashVal % table->num_buckets;

  pthread_rwlock_wrlock(&table->buckets[index].rwlock);

  hashRecord* current = table->buckets[index].head;
  while (current != NULL) {
    if (current->hash == hashVal) {
      printf("Insert failed. Entry %d is a duplicate.", hashVal);
      pthread_rwlock_unlock(&table->buckets[index].rwlock);
      return;
    }
    current = current->next;
  }

  hashRecord* rec = (hashRecord*) malloc(sizeof(hashRecord));
  strcpy(rec->name, name);
  rec->salary = salary;
  rec->hash = hashVal;

  rec->next = table->buckets[index].head;
  table->buckets[index].head = rec;

  pthread_rwlock_unlock(&table->buckets[index].rwlock);

  printf("Inserted %d,%s,%d\n", hashVal, name, salary);
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
      printf("Deleted record for %d,%s,%d\n", hashVal, name, current->salary);
      free(current);
      break;
    }
    prev = current;
    current = current->next;
  }

  pthread_rwlock_unlock(&table->buckets[index].rwlock);

  if (!found) {
    printf("Entry %d not deleted. Not in database.\n", hashVal);
  }
}
