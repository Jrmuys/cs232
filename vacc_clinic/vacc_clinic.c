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

// Different name for walk for realism and clarity
void wait(int lower, int upper)
{
    walk(lower, upper);
}

/*
 * Add to buffer:
 * Paramater: station_id, an int to be saved to the buffer
 * Save the station_id to the buffer and increment the "next_in" variable
 */
void add_to_buffer(int station_id)
{
    station_assignment_buffer[station_assignment_next_in] = station_id;
    station_assignment_next_in++;
    station_assignment_next_in %= NUM_STATIONS;
}


/*
 * Remove from buffer:
 * Gets an item from the buffer, increments the "next out" variable, and returns the item
 */
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

        /************ VIAL PICKUP ***********/

        fprintf(stderr, "%s: nurse %ld is walking to get a vial\n", curr_time_s(), id);

        // Walk to get vial
        walk(1, 3);

        fprintf(stderr, "%s: nurse %ld is walking to get a vial\n", curr_time_s(), id);

        // Wait for the vials to be unlocked (CRITICAL SECTION START)
        if (sem_wait(&vial_mutex) == -1)
        {
            fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore vial_mutex", 
                    curr_time_s(), id);
            exit(1);
        };

        // Check that there are vials left
        if (num_vials_left <= 0)
        {
            // If not, release the mutex...
            if (sem_post(&vial_mutex) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore vial_mutex", 
                        curr_time_s(), id);
                exit(1);
            }
            
            // ... and leave the clinic
            fprintf(stderr, "%s: nurse %ld is done\n", curr_time_s(), id);
            pthread_exit(NULL);
        } // Else there is a vial, so continue

        num_vials_left--; // Reduce the count of vials by one (nurse is taking a vial)
        fprintf(stderr, "%s: nurse %ld has obtained a vial\n", curr_time_s(), id);

        // Unlock the vials (CRITICAL SECTION END)
        if (sem_post(&vial_mutex) == -1)
        {
            fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore vial_mutex", 
                    curr_time_s(), id);
            exit(1);
        }

        // Walk back to station
        walk(1, 3);

        /************ VACCINATION ***********/

        fprintf(stderr, "%s: nurse %ld is back at their station\n", curr_time_s(), id);

        // Do a round of vaccinations for each shot in the vial
        for (int shot = 0; shot < SHOTS_PER_VIAL; shot++)
        {

            fprintf(stderr, "%s: nurse %ld is ready to give shot %i out of %i\n", curr_time_s(), id, shot + 1, SHOTS_PER_VIAL);

            // Wait for the queue not to be empty
            if (sem_wait(&station_assignment_empty) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore station_assignment_empty", 
                        curr_time_s(), id);
                exit(1);
            };

            // Lock the station assignment mutex (CRITICAL SECTION START)
            if (sem_wait(&station_assignment_mutex) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore station_assignment_mutex", 
                        curr_time_s(), id);
                exit(1);
            };

            // Add station id to buffer (protected because it's in a critical section)
            add_to_buffer(station_id);

            // Unlock the station assignment mutex (CRITICAL SECTION END)
            if (sem_post(&station_assignment_mutex) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore station_assignment_mutex", 
                        curr_time_s(), id);
                exit(1);
            }

            // Signal that the queue is not full
            if (sem_post(&station_assignment_full) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore station_assignment_full", 
                        curr_time_s(), id);
                exit(1);
            }

            fprintf(stderr, "%s: nurse %ld waiting for client to arrive\n", curr_time_s(), id);

            // Wait for client to be ready for vaccine
            if (sem_wait(&vaccination_ready[station_id]) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore vaccination_ready[%i]", 
                        curr_time_s(), id, station_id);
                exit(1);
            };

            fprintf(stderr, "%s: nurse %ld sees that the client is ready for the vaccination\n", curr_time_s(), id);

            // Giving the vaccine takes 5 seconds
            wait(5, 5);

            // Signal that you have completed giving the vaccine
            if (sem_post(&vaccination_complete[station_id]) == -1)
            {
                fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore vaccination_complete[%i]", 
                        curr_time_s(), id, station_id);
                exit(1);
            }

            fprintf(stderr, "%s: nurse %ld has given the vaccination to the client\n", curr_time_s(), id);
        }
    }
}

void *client(void *arg)
{
    long int id = (long int)arg;

    /************ REGISTRATION ***********/

    fprintf(stderr, "%s: client %ld has arrived and is walking to register\n",
            curr_time_s(), id);

    // Walk to register
    walk(3, 10);

    fprintf(stderr, "%s: client %ld waiting to register\n",
            curr_time_s(), id);

    // Wait for a registration table to open up
    if (sem_wait(&registration) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore registration", 
                curr_time_s(), id);
        exit(1);
    };

    // Register (takes between 3 and 10 seconds)
    fprintf(stderr, "%s: client %ld is registering\n",
            curr_time_s(), id);
    wait(3, 10);

    // Signal that the client is done registering
    if (sem_post(&registration) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore SEMNAME", 
                curr_time_s(), id);
        exit(1);
    }

    /************ STATION ASSIGNMENT ***********/

    // Walk to the station assignment queue
    fprintf(stderr, "%s: client %ld is done registering and will walk to the station assignment queue\n",
            curr_time_s(), id);

    walk(3, 10);


    // Station assignment:
    fprintf(stderr, "%s: client %ld waiting to receive a station assignment\n",
            curr_time_s(), id);

    // Wait for station assignment queue to not be full
    if (sem_wait(&station_assignment_full) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore station_assignment_full", 
                curr_time_s(), id);
        exit(1);
    };

    // Lock the station assignment buffer
    if (sem_wait(&station_assignment_mutex) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore station_assignment_mutex", 
                curr_time_s(), id);
        exit(1);
    };

    // Get the next station id from the buffer
    int station_id = remove_from_buffer();

    // Release the lock on the station assignment buffer
    if (sem_post(&station_assignment_mutex) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore station_assignment_mutex", 
                curr_time_s(), id);
        exit(1);
    }

    // Signal that the station assignment queue is emtpty(er)
    if (sem_post(&station_assignment_empty) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore station_assignment_empty", 
                curr_time_s(), id);
        exit(1);
    }
    fprintf(stderr, "%s: client %ld got assigned to station %i\n",
            curr_time_s(), id, station_id);

    // Walk to the assigned station
    walk(1, 2);

    /************ VACCINATION ***********/

    fprintf(stderr, "%s: client %ld has arrived at station %i \n",
            curr_time_s(), id, station_id);

    // Signal that you are ready to receive the vaccine
    if (sem_post(&vaccination_ready[station_id]) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while posting to semaphore vaccination_ready[%i]", 
                curr_time_s(), id, station_id);
        exit(1);
    }

    fprintf(stderr, "%s: client %ld is ready to receive the vaccination \n",
            curr_time_s(), id);

    // Wait for nurse to complete the vaccine
    if (sem_wait(&vaccination_complete[station_id]) == -1)
    {
        fprintf(stderr, "%s: Client %ld experienced error while waiting for semaphore vaccination_complete[%i]", 
                curr_time_s(), id, station_id);
        exit(1);
    };
    fprintf(stderr, "%s: client %ld has received the vaccination \n",
            curr_time_s(), id);

    fprintf(stderr, "%s: client %ld leaves the clinic!\n", curr_time_s(), id);

    pthread_exit(NULL); // End the thread (complete)
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

    // Initialize client and nurse threads
    pthread_t client_thr[NUM_CLIENTS], nurse_thr[NUM_CLIENTS];

    // Create nurse threads
    for (int n = 0; n < NUM_NURSES; n++)
    {
        if (pthread_create(&nurse_thr[n], NULL, nurse, (void *)(n)) == -1) {
            fprintf(stderr, "Error creating thread nurse_thr[%i]", n);
            exit(1);
        }
    }

    // Create client threads
    for (int c = 0; c < NUM_CLIENTS; c++)
    {
        if (pthread_create(&client_thr[c], NULL, client, (void *)(c)) == -1) {
            fprintf(stderr, "Error creating thread client_thr[%i]", c);
            exit(1);
        }
        wait(0, 1); // Wait random time between thread creations
    }

    pthread_exit(0); // Cleanup pthreads

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