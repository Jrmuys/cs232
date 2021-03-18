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
// #define NUM_CLIENTS 2

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
int station_assignment_buffer[NUM_STATIONS];
int station_assignment_next_in;
int station_assignment_next_out;

sem_t vaccination_ready[NUM_STATIONS];
sem_t vaccination_complete[NUM_STATIONS];

int num_vials_left = NUM_VIALS;

int get_rand_in_range(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

char *time_str;
char *curr_time_s()
{
    time_t t;
    time(&t);
    time_str = ctime(&t);
    // replace ending newline with end-of-string.
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

// lower and upper are in seconds.
void walk(int lower, int upper)
{
    // Sleep for random range between lower and upper, converted to microseconds for usleep
    usleep(get_rand_in_range(lower * MICROSECONDS_PER_SECOND, upper * MICROSECONDS_PER_SECOND));
}

void wait(int lower, int upper)
{
    walk(lower, upper);
}

void add_to_buffer(int station_id)
{
    station_assignment_buffer[station_assignment_next_in] = station_id;
    station_assignment_next_in++;
    station_assignment_next_in %= NUM_STATIONS;
}

int remove_from_buffer()
{
    int station_id = station_assignment_buffer[station_assignment_next_out];
    station_assignment_next_out++;
    station_assignment_next_out %= NUM_STATIONS;
    return station_id;
}

// arg is the nurses station number.
void *nurse(void *arg)
{
    long int id = (long int)arg;
    int station_id = id;

    fprintf(stderr, "%s: nurse %ld started \n", curr_time_s(), id);

    while (1)
    {

        fprintf(stderr, "%s: nurse %ld is walking to get a vial\n", curr_time_s(), id);

        // Walk to get vial
        walk(1, 3);

        sem_wait(&vial_mutex);
        if (num_vials_left <= 0)
        {
            sem_post(&vial_mutex);
            fprintf(stderr, "%s: nurse %ld is done\n", curr_time_s(), id);

            pthread_exit(NULL);
        }
        num_vials_left--;
        fprintf(stderr, "%s: nurse %ld has obtained a vial\n", curr_time_s(), id);
        sem_post(&vial_mutex);

        // walk back to station
        walk(1, 3);

        for (int shot = 0; shot < SHOTS_PER_VIAL; shot++)
        {

            fprintf(stderr, "%s: nurse %ld is ready to give shot %i out of %i\n", curr_time_s(), id, shot + 1, SHOTS_PER_VIAL);

            sem_wait(&station_assignment_empty);
            sem_wait(&station_assignment_mutex);
            add_to_buffer(station_id);
            sem_post(&station_assignment_mutex);
            sem_post(&station_assignment_full);

            fprintf(stderr, "%s: nurse %ld waiting for client to arrive\n", curr_time_s(), id);

            // Wait for client to be ready for vaccine
            sem_wait(&vaccination_ready[station_id]);

            fprintf(stderr, "%s: nurse %ld sees that the client is ready for the vaccination\n", curr_time_s(), id);

            wait(5, 5);
            // Signal that you have completed giving the vaccine
            sem_post(&vaccination_complete[station_id]);

            fprintf(stderr, "%s: nurse %ld has given the vaccination to the client\n", curr_time_s(), id);
        }
    }
}

void *client(void *arg)
{
    long int id = (long int)arg;

    fprintf(stderr, "%s: client %ld has arrived and is walking to register\n",
            curr_time_s(), id);

    // Walk to register
    walk(3, 10);

    fprintf(stderr, "%s: client %ld waiting to register\n",
            curr_time_s(), id);

    sem_wait(&registration);

    fprintf(stderr, "%s: client %ld is registering\n",
            curr_time_s(), id);
    wait(3, 10);

    sem_post(&registration);

    fprintf(stderr, "%s: client %ld is done registering and will walk to the registration queue\n",
            curr_time_s(), id);

    walk(3, 10);

    fprintf(stderr, "%s: client %ld waiting to receive a station assignment\n",
        curr_time_s(), id);

    sem_wait(&station_assignment_full);
    sem_wait(&station_assignment_mutex);
    int station_id = remove_from_buffer();
    sem_post(&station_assignment_mutex);
    sem_post(&station_assignment_empty);

    fprintf(stderr, "%s: client %ld got assigned to station %i\n",
            curr_time_s(), id, station_id);

    walk(1, 2);

    fprintf(stderr, "%s: client %ld has arrived at station %i \n",
            curr_time_s(), id, station_id);
    // Signal that you are ready to receive the vaccine

    sem_post(&vaccination_ready[station_id]);

    fprintf(stderr, "%s: client %ld is ready to receive the vaccination \n",
            curr_time_s(), id);
    // Wait for nurse to complete the vaccine
    sem_wait(&vaccination_complete[station_id]);
    fprintf(stderr, "%s: client %ld has received the vaccination \n",
            curr_time_s(), id);

    fprintf(stderr, "%s: client %ld leaves the clinic!\n", curr_time_s(), id);

    pthread_exit(NULL);
}

int main()
{
    srand(time(0));

    // Initialize all semaphores

    sem_init(&registration, 0, NUM_REGISTRATIONS_SIMULTANEOUSLY);
    sem_init(&vial_mutex, 0, 1);
    // Producer consumer semaphores for station assignment
    sem_init(&station_assignment_mutex, 0, 1);
    sem_init(&station_assignment_empty, 0, NUM_STATIONS);
    sem_init(&station_assignment_full, 0, 0);
    // Vaccination rendezvous semaphores

    for (int i = 0; i < NUM_STATIONS; i++)
    {
        sem_init(&vaccination_ready[i], 0, 0);
        sem_init(&vaccination_complete[i], 0, 0);
    }

    // Test Threads

    // pthread_t client1, nurse1;
    // pthread_create(&client1, NULL, client, (void*)&client);
    // pthread_create(&nurse1, NULL, nurse, (void*)&nurse);

    pthread_t client_thr[NUM_CLIENTS], nurse_thr[NUM_CLIENTS];
    int id[NUM_CLIENTS];

    // Create nurse threads
    for (int n = 0; n < NUM_NURSES; n++)
    {
        id[n] = n + 1;
        pthread_create(&nurse_thr[n], NULL, nurse, (void *)(n));
    }

    // Create client threads
    for (int c = 0; c < NUM_CLIENTS; c++)
    {
        id[c] = c + 1;

        pthread_create(&client_thr[c], NULL, client, (void *)(c));
        wait(0, 1);
    }

    pthread_exit(0);

    // Done with program, destory all semaphores

    sem_destroy(&registration);
    sem_destroy(&vial_mutex);
    sem_destroy(&station_assignment_mutex);
    sem_destroy(&station_assignment_empty);
    sem_destroy(&station_assignment_full);
    for (int i = 0; i < NUM_STATIONS; i++)
    {
        sem_destroy(&vaccination_ready[i]);
        sem_destroy(&vaccination_complete[i]);
    }
}