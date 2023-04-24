#include <pthread.h>

struct station {
	pthread_mutex_t lock; //mutex lock
    pthread_cond_t train_arrival; //to indicate that a train has arrived now
    pthread_cond_t train_is_good_to_go; //to indicate that the train is ready to leave now

    int number_of_waiting_passengers; 
    int number_of_available_seats;
    int number_of_remaining_seats;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);