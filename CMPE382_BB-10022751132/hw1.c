#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>



int is_prime(int num) {
    if (num <= 1) return 0;
    for (int i = 2; i <= sqrt(num); i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

void generate_file(const char *filename, int N, int XXX) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < N; i++) {
        fprintf(file, "%d\n", (rand() % XXX ) + 1);
    }
    fclose(file);
}

int count_primes(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    int num, count = 0;
    while (fscanf(file, "%d", &num) != EOF) {
        if (is_prime(num)) count++;
    }
    fclose(file);
    return count;
}

int main() {
    int student_id = 10022751132;
    int XXX =(student_id % 1000)+100;

    int N;
    printf("Enter N: ");
    scanf("%d", &N);
    srand(time(NULL));
    
    generate_file("File1", N, XXX);
    generate_file("File2", N, XXX);

    int parent_to_child1[2], child1_to_parent[2];
    int parent_to_child2[2], child2_to_parent[2];
    int result_pipe1[2], result_pipe2[2];
    
    pipe(parent_to_child1);
    pipe(child1_to_parent);
    pipe(parent_to_child2);
    pipe(child2_to_parent);
    
    
    
    pipe(result_pipe1); //In this case, I try to solve "I am Child process Px"... Children communicate with each other...
    pipe(result_pipe2); //In this case, I try to solve "I am Child process Px"... Children communicate with each other...

    pid_t pid1 = fork();
    if (pid1 == 0) { // Child 1
        close(parent_to_child1[1]);
        close(child1_to_parent[0]);
        close(result_pipe1[1]);
        
        read(parent_to_child1[0], &N, sizeof(N));
        int pn1 = count_primes("File1");
        write(child1_to_parent[1], &pn1, sizeof(pn1));


        
        int pn2;
        read(result_pipe1[0], &pn2, sizeof(pn2));
        printf("I am Child process P1: The winner is child process P%d\n", (pn1 > pn2) ? 1 : 2);
        
        exit(0);
    } else {
        pid_t pid2 = fork();
        
        if (pid2 == 0) { // Child 2
            close(parent_to_child2[1]);
            close(child2_to_parent[0]);
            close(result_pipe2[1]);
            
            read(parent_to_child2[0], &N, sizeof(N));
            int pn2 = count_primes("File2");
            write(child2_to_parent[1], &pn2, sizeof(pn2));
            
            int pn1;
            read(result_pipe2[0], &pn1, sizeof(pn1));
            printf("I am Child process P2: The winner is child process P%d\n", (pn2 > pn1) ? 2 : 1);
            
            exit(0);
        } else { // Parent
            close(parent_to_child1[0]);
            close(parent_to_child2[0]);
            close(child1_to_parent[1]);
            close(child2_to_parent[1]);
            
            write(parent_to_child1[1], &N, sizeof(N));
            write(parent_to_child2[1], &N, sizeof(N));
            
            int pn1, pn2;
            read(child1_to_parent[0], &pn1, sizeof(pn1));
            read(child2_to_parent[0], &pn2, sizeof(pn2));
            
            write(result_pipe1[1], &pn2, sizeof(pn2));
            write(result_pipe2[1], &pn1, sizeof(pn1));
            
            wait(NULL);
            wait(NULL);
            
            printf("\nThe number of positive integers in each file: %d\n", N);
            printf("The number of prime numbers in File1: %d\n", pn1);
            printf("The number of prime numbers in File2: %d\n", pn2);
        }
    }
    return 0;
}