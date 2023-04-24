#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "caltrain.h"

void station_init(struct station *station)
{

	if (pthread_mutex_init(&(station->lock), NULL) != 0)
	{
		perror("Error initializing the mutex");
		exit(1);
	}

	if (pthread_cond_init(&(station->train_arrival), NULL) != 0)
	{
		perror("Error initializing the \"train_arrived\" condition variable");
		exit(1);
	}

	if (pthread_cond_init(&(station->train_is_good_to_go), NULL) != 0)
	{
		perror("Error initializing the \"train_is_good_to_go\" condition variable");
		exit(1);
	}

	station->number_of_available_seats = 0;
	station->number_of_waiting_passengers = 0;
	station->number_of_remaining_seats = 0;
}

void station_load_train(struct station *station, int count)
{
	// we put this here because there can be only one train
	station->number_of_available_seats = count;

	pthread_mutex_lock(&(station->lock));
	station->number_of_remaining_seats = count;
	// if there's no avalible seats or there's no passengers waiting then leave promptly
	if (station->number_of_waiting_passengers != 0 && station->number_of_available_seats != 0)
	{
		// here means that we have passengers and avalable seats
		// then we broadcast all the waiting passengers that a train is here
		pthread_cond_broadcast(&(station->train_arrival));
		// then we need to block this train and make it wait until the train is good to go
		// and that happens when there's no av seats left or no passenegers waiting
		pthread_cond_wait(&(station->train_is_good_to_go), &(station->lock));
	}
	pthread_mutex_unlock(&(station->lock));
}

void station_wait_for_train(struct station *station)
{
	pthread_mutex_lock(&(station->lock));
	// increment the number of waiting passenegers by one to indicate that there's a passenger now
	station->number_of_waiting_passengers++;
	do
	{
		// while there's no enough seats(train that does not have enough seats or no train is here yet)
		// then we wait for a train arrival(with available seats)
		pthread_cond_wait(&(station->train_arrival), &(station->lock));
		station->number_of_remaining_seats--;
	} while (station->number_of_remaining_seats < 0);

	// if we're here then this indicates that we got out of the polling loop and there's at least one seat

	pthread_mutex_unlock(&(station->lock));
	// here the robot calls the on board function after the passenger is seated
}

void station_on_board(struct station *station)
{
	pthread_mutex_lock(&(station->lock));
	if (station->number_of_waiting_passengers != 0 && station->number_of_available_seats != 0)
	{
		// decerement both when the passenger is seated
		station->number_of_available_seats--;
		station->number_of_waiting_passengers--;
	}
	pthread_mutex_unlock(&(station->lock));
	// since we have only one train at a time then the train threads can not interleave
	// it means that the current train must leave before the next is here
	// so we can put this portion of code right here
	if (station->number_of_available_seats == 0 || station->number_of_waiting_passengers == 0)
	{
		// if there's no passengers here or there's no any seats
		// then we signal(since we can only have one train) the train that it's good to go
		pthread_cond_signal(&(station->train_is_good_to_go));
	}
}
