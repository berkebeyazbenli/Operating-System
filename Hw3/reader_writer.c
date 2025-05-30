#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define MAX_PARTICIPANTS 9
#define SHARED_MEMORY_SIZE 1
#define ITERATIONS 5

// Shared resources
int shared_memory = 0;
sem_t resource_lock;      // Controls access to shared memory
sem_t writer_lock;        // Controls writer access
int active_readers = 0;   // Tracks concurrent readers

// Authentication system
typedef struct {
    unsigned long auth_token;
    int is_authenticated;
} AuthRecord;

AuthRecord auth_database[MAX_PARTICIPANTS * 2];
int auth_records_count = 0;

// Thread configuration
typedef struct {
    int participant_id;
    int is_authenticated;
    int is_reader;
} ThreadConfig;

// Generate authentication token
unsigned long generate_auth_token(pthread_t thread) {
    return (unsigned long)(uintptr_t)thread;
}

// Register participant in authentication system
int register_participant(pthread_t thread) {
    sem_wait(&resource_lock);
    if (auth_records_count >= MAX_PARTICIPANTS * 2) {
        sem_post(&resource_lock);
        return 0;
    }
    auth_database[auth_records_count].auth_token = generate_auth_token(thread);
    auth_database[auth_records_count].is_authenticated = 1;
    auth_records_count++;
    sem_post(&resource_lock);
    return 1;
}

// Verify participant authentication
int verify_authentication(pthread_t thread) {
    unsigned long token = generate_auth_token(thread);
    for (int i = 0; i < auth_records_count; i++) {
        if (auth_database[i].auth_token == token && auth_database[i].is_authenticated) {
            return 1;
        }
    }
    return 0;
}

// Reading participant function
void* reading_participant(void* config) {
    ThreadConfig* cfg = (ThreadConfig*)config;
    pthread_t current_thread = pthread_self();
    
    for (int iteration = 0; iteration < ITERATIONS; iteration++) {
        usleep(1000000);  // 1 second delay
        
        if (cfg->is_authenticated) {
            // Authenticated reader implementation
            sem_wait(&resource_lock);
            active_readers++;
            if (active_readers == 1) {
                sem_wait(&writer_lock);
            }
            sem_post(&resource_lock);
            
            // Perform reading operation
            int current_value = shared_memory;
            printf("Participant_ID: %d\tAuth_Token: %lu\tStatus: authenticated\tType: reader\tValue: %d\n",
                   cfg->participant_id, generate_auth_token(current_thread), current_value);
            
            sem_wait(&resource_lock);
            active_readers--;
            if (active_readers == 0) {
                sem_post(&writer_lock);
            }
            sem_post(&resource_lock);
        } else {
            // Unauthenticated reader implementation
            printf("Participant_ID: %d\tAuth_Token: %lu\tStatus: unauthenticated\tType: reader\tValue: %d\n",
                   cfg->participant_id, generate_auth_token(current_thread), shared_memory);
        }
    }
    
    free(cfg);
    return NULL;
}

// Writing participant function
void* writing_participant(void* config) {
    ThreadConfig* cfg = (ThreadConfig*)config;
    pthread_t current_thread = pthread_self();
    
    for (int iteration = 0; iteration < ITERATIONS; iteration++) {
        usleep(1000000);  // 1 second delay
        
        if (cfg->is_authenticated) {
            // Authenticated writer implementation
            sem_wait(&writer_lock);
            
            // Perform writing operation
            int new_value = rand() % 10000;
            shared_memory = new_value;
            printf("Participant_ID: %d\tAuth_Token: %lu\tStatus: authenticated\tType: writer\tValue: %d\n",
                   cfg->participant_id, generate_auth_token(current_thread), new_value);
            
            sem_post(&writer_lock);
        } else {
            // Unauthenticated writer implementation
            int new_value = rand() % 10000;
            shared_memory = new_value;
            printf("Participant_ID: %d\tAuth_Token: %lu\tStatus: unauthenticated\tType: writer\tValue: %d\n",
                   cfg->participant_id, generate_auth_token(current_thread), new_value);
        }
    }
    
    free(cfg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <number_of_readers> <number_of_writers>\n", argv[0]);
        return 1;
    }
    
    int reader_count = atoi(argv[1]);
    int writer_count = atoi(argv[2]);
    
    if (reader_count < 1 || reader_count > 9 || writer_count < 1 || writer_count > 9) {
        printf("Number of participants must be between 1 and 9\n");
        return 1;
    }
    
    // Initialize synchronization primitives
    sem_init(&resource_lock, 0, 1);
    sem_init(&writer_lock, 0, 1);
    
    // Allocate thread arrays
    pthread_t* reader_threads = malloc((reader_count * 2) * sizeof(pthread_t));
    pthread_t* writer_threads = malloc((writer_count * 2) * sizeof(pthread_t));
    
    // Initialize authenticated readers
    for (int i = 0; i < reader_count; i++) {
        ThreadConfig* cfg = malloc(sizeof(ThreadConfig));
        cfg->participant_id = i + 1;
        cfg->is_authenticated = 1;
        cfg->is_reader = 1;
        pthread_create(&reader_threads[i], NULL, reading_participant, cfg);
        register_participant(reader_threads[i]);
    }
    
    // Initialize unauthenticated readers
    for (int i = 0; i < reader_count; i++) {
        ThreadConfig* cfg = malloc(sizeof(ThreadConfig));
        cfg->participant_id = i + 1;
        cfg->is_authenticated = 0;
        cfg->is_reader = 1;
        pthread_create(&reader_threads[i + reader_count], NULL, reading_participant, cfg);
    }
    
    // Initialize authenticated writers
    for (int i = 0; i < writer_count; i++) {
        ThreadConfig* cfg = malloc(sizeof(ThreadConfig));
        cfg->participant_id = i + 1;
        cfg->is_authenticated = 1;
        cfg->is_reader = 0;
        pthread_create(&writer_threads[i], NULL, writing_participant, cfg);
        register_participant(writer_threads[i]);
    }
    
    // Initialize unauthenticated writers
    for (int i = 0; i < writer_count; i++) {
        ThreadConfig* cfg = malloc(sizeof(ThreadConfig));
        cfg->participant_id = i + 1;
        cfg->is_authenticated = 0;
        cfg->is_reader = 0;
        pthread_create(&writer_threads[i + writer_count], NULL, writing_participant, cfg);
    }
    
    // Synchronize thread completion
    for (int i = 0; i < reader_count * 2; i++) {
        pthread_join(reader_threads[i], NULL);
    }
    for (int i = 0; i < writer_count * 2; i++) {
        pthread_join(writer_threads[i], NULL);
    }
    
    // Resource cleanup
    free(reader_threads);
    free(writer_threads);
    sem_destroy(&resource_lock);
    sem_destroy(&writer_lock);
    
    return 0;
} 