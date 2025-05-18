#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>

#define MAX_PATH 256

sem_t thread_sem;

typedef struct {
    char filepath[MAX_PATH];
    int thread_id;
    struct timeval start_time;
} ThreadArgs;

int is_prime(int num) {
    if (num <= 1) return 0;
    if (num == 2) return 1;
    if (num % 2 == 0) return 0;
    for (int i = 3; i * i <= num; i += 2)
        if (num % i == 0)
            return 0;
    return 1;
}

void* process_file(void* arg) {
    sem_wait(&thread_sem);
    ThreadArgs* args = (ThreadArgs*)arg;
    
    struct timeval thread_start, thread_end;
    gettimeofday(&thread_start, NULL);
    
    FILE* fp = fopen(args->filepath, "r");
    if (!fp) {
        perror("File open error");
        sem_post(&thread_sem);
        pthread_exit(NULL);
    }

    int count = 0, num;
    while (fscanf(fp, "%d", &num) == 1) {
        if (is_prime(num))
            count++;
    }
    fclose(fp);

    gettimeofday(&thread_end, NULL);
    double thread_time = (thread_end.tv_sec - thread_start.tv_sec) + 
                       (thread_end.tv_usec - thread_start.tv_usec) / 1000000.0;

    printf("Thread %d found %d primes in %s (%.6f seconds)\n", 
           args->thread_id, count, args->filepath, thread_time);
    
    free(arg);
    sem_post(&thread_sem);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <thread_limit>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct timeval program_start, program_end;
    gettimeofday(&program_start, NULL);

    char* dir_name = argv[1];
    int max_threads = atoi(argv[2]);

    sem_init(&thread_sem, 0, max_threads);

    DIR* dir = opendir(dir_name);
    if (!dir) {
        perror("Directory open error");
        exit(EXIT_FAILURE);
    }

    struct dirent* entry;
    pthread_t threads[1000];
    int thread_count = 0;

    printf("Starting processing with %d threads...\n", max_threads);
    
    while ((entry = readdir(dir)) != NULL) {
        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_name, entry->d_name);
        
        struct stat path_stat;
        stat(fullpath, &path_stat);
        
        if (S_ISREG(path_stat.st_mode)) {
            ThreadArgs* args = malloc(sizeof(ThreadArgs));
            strncpy(args->filepath, fullpath, MAX_PATH);
            args->thread_id = thread_count + 1;
            gettimeofday(&args->start_time, NULL);
            pthread_create(&threads[thread_count++], NULL, process_file, args);
        }
    }
    closedir(dir);

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&program_end, NULL);
    double total_time = (program_end.tv_sec - program_start.tv_sec) + 
                       (program_end.tv_usec - program_start.tv_usec) / 1000000.0;

    printf("\nTotal execution time: %.6f seconds\n", total_time);
    printf("Processed %d files with %d threads\n", thread_count, max_threads);

    sem_destroy(&thread_sem);
    return 0;
}