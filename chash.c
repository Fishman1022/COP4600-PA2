#include "parser.h"
#include "threadlock.h"
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>



typedef struct hash_struct
{
  uint32_t hash;
  char name[50];
  uint32_t salary;
  struct hash_struct *next;
} hashRecord;

//Organizes Hash records into an array, lets us track if one is locked/released
typedef struct {
    hashRecord *head;
    pthread_mutex_t lock;
} hashBucket;

//Hash Table Wrapper
typedef struct {
    hashBucket *buckets;
    size_t num_buckets;
} concurrentHashTable;

concurrentHashTable* createHashTable(size_t num_buckets){
    //Creates table, then initializes num buckets and creates array structure for each bucket
    concurrentHashTable* table = malloc(sizeof(concurrentHashTable));
    table->num_buckets = num_buckets;
    table->buckets = malloc(num_buckets * sizeof(hashBucket));

    //Initializes each bucket
    for (int i = 0; i < num_buckets; i++)
    {
        table->buckets->head = NULL;
        pthread_mutex_init(&table->buckets[i].lock, NULL);
    }

    return table;
}

//These are template, you can change them to whateverr yoou think wroks best lol
void readCommandsFile("commands.txt");
void threadRelease(concurrentHashTable* table);
void threadLock(concurrentHashTable* table);
void hashInsert(concurrentHashTable* table, char* name, int salary, int priority);
void hashDelete(concurrentHashTable * table, char* name,  int priority);
void hashSearch(concurrentHashTable* table, char* name, int priority);
void hashPrint(concurrentHashTable* table);
void writeLogFile();


int main() {

  concurrentHashTable* table = createHashTable(100);
}