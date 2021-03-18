


/*
 * Vaccine Clinic
 * 
 * 
 * Author: Joel Muyskens
 * For: CS 232, Fa 2021
 *      Homework 5
 * 
 * Date: 03/17/2021
 * 
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_VIALS 30
#define SHOTS_PER_VIAL 6
#define NUM_CLIENTS (NUM_VIALS * SHOTS_PER_VIAL)
#define NUM_NURSES 10
#define NUM_STATIONS NUM_NURSES
#define NUM_REGISTRATIONS_SIMULTANEOUSLY 4
#define MICROSECONDS_PER_SECOND 1000000

/* global variables */

sem_t registration;
sem_t vial_mutex;

sem_t station_assignment_mutex;
sem_t station_assignment_empty;
sem_t station_assignment_full;
int station_assignment_count;

sem_t vaccination_ready;
sem_t vaccination_complete;


int num_vials_left = 1;



int get_rand_in_range(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

char *time_str;
char *curr_time_s() {
    time_t t;
    time(&t);
    time_str = ctime(&t);
    // replace ending newline with end-of-string.
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

// lower and upper are in seconds.
void walk(int lower, int upper) {
    // Sleep for random range between lower and upper, converted to microseconds for usleep
    usleep(get_rand_in_range(lower * MICROSECONDS_PER_SECOND , upper * MICROSECONDS_PER_SECOND));
}

// arg is the nurses station number.
void *nurse(void *arg) {
    long int id = (long int)arg;

    fprintf(stderr, "%s: nurse %ld started\n", curr_time_s(), id);

    fprintf(stderr, "%s: nurse %ld waiting for client to arrive\n", curr_time_s(), id);

    // Wait for client to be ready for vaccine
    sem_wait(&vaccination_ready);

    fprintf(stderr, "%s: nurse %ld sees that the client is ready for the vaccination\n", curr_time_s(), id);

    // Signal that you have completed giving the vaccine
    sem_post(&vaccination_complete);

    fprintf(stderr, "%s: nurse %ld has given the vaccination to the client\n", curr_time_s(), id);


    fprintf(stderr, "%s: nurse %ld is done\n", curr_time_s(), id);

    pthread_exit(NULL);
}

void *client(void *arg) {
    long int id = (long int)arg;

    fprintf(stderr, "%s: client %ld has arrived and is walking to register\n",
            curr_time_s(), id);

    // Walk to register


    // fprintf(stderr, "%s: client %ld waiting to register\n",
    //         curr_time_s(), id);

    // fprintf(stderr, "%s: client %ld is registering\n",
    //         curr_time_s(), id);

    // fprintf(stderr, "%s: client %ld is done registering\n",
    //         curr_time_s(), id);

    // fprintf(stderr, "%s: client %ld got assigned to station\n",
    //         curr_time_s(), id);

    fprintf(stderr, "%s: client %ld has arrived at station TODO: add number \n",
            curr_time_s(), id);
    // Signal that you are ready to receive the vaccine

    sem_post(&vaccination_ready);


    fprintf(stderr, "%s: client %ld is ready to receive the vaccination \n",
            curr_time_s(), id);
    // Wait for nurse to complete the vaccine
    sem_wait(&vaccination_complete);
    fprintf(stderr, "%s: client %ld has received the vaccination \n",
        curr_time_s(), id);

    fprintf(stderr, "%s: client %ld leaves the clinic!\n", curr_time_s(), id);

    pthread_exit(NULL);
}

/*
sem_t registration;
sem_t vial;

sem_t station_assignment_mutex;
sem_t station_assignment_empty;
sem_t station_assignment_full;

sem_t vaccination_ready;
sem_t vaccination_complete;
 * 
 */
int main() {
    srand(time(0));

    // Initialize all semaphores

    sem_init(&registration, 0, 4);
    sem_init(&vial_mutex, 0, 1);
    // Producer consumer semaphores for station assignment
    sem_init(&station_assignment_mutex, 0, 1);
    sem_init(&station_assignment_empty, 0, NUM_STATIONS);
    sem_init(&station_assignment_full, 0, 0);
    // Vaccination rendezvous semaphores
    sem_init(&vaccination_ready, 0, 0);
    sem_init(&vaccination_complete, 0, 0);


    // Test Threads
    int id[1];
    id[0] = 1;

    pthread_t client1, nurse1;
    pthread_create(&client1, NULL, client, (void*)&());
    pthread_create(&nurse1, NULL, nurse, (void*)&id[0]);

    // pthread_t clients[NUM_CLIENTS], nurses[NUM_NURSES];
    
    // // Create client threads
    // for (int c = 0; c < NUM_CLIENTS; c++) {
    //     pthread_create(clients[c], NULL, client, (void *)&clients[c]);
    // }

    // // Create nurse threads
    // for (int n = 0; n < NUM_NURSES; n++) {
    //     pthreads_create(nurses[n], NULL, nurse, (void *)&nurses[n]);
    // }

    pthread_exit(0);


    // Done with program, destory all semaphores

    sem_destroy(&registration);
    sem_destroy(&vial_mutex);
    sem_destroy(&station_assignment_mutex);
    sem_destroy(&station_assignment_empty);
    sem_destroy(&station_assignment_full);
    sem_destroy(&vaccination_ready);
    sem_destroy(&vaccination_complete);
}
