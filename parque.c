#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_LENGTH 15
#define CONTROL_VEHICLE_ID -1
#define PARK_OPEN 0
#define PARK_CLOSED 1
#define PARK_FULL 2
#define PARKING_VEHICLE 3
#define ENTERING_VEHICLE 4


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//number of park spaces of the park
int capacity;

//park's state(0 if open, 1 if closed, 2 if full)
int state;
int parkstate;

//space of the park that is unavailable(= number of cars in the park)
int unavailable;

int open_time;

typedef enum {NORTH, SOUTH, EAST, WEST} Direction;

typedef struct{
	int id;
	float p_time;
	char fifo[FIFO_LENGTH];
	Direction dir;
} Vehicle;

void *v_controller(void* arg){
	void* ret = NULL;
	Vehicle vehicle = *(Vehicle*) arg;
	int fd;
	parkstate = ENTERING_VEHICLE;

	fd = open(vehicle.fifo, O_WRONLY);

	//locks all other threads and controls number of available spaces in the park
	pthread_mutex_lock(&mutex);

	if (unavailable < capacity && state == PARK_OPEN){
		unavailable++;
		pthread_mutex_unlock(&mutex);
		parkstate = PARKING_VEHICLE;
		usleep(vehicle.p_time * 1000);
		unavailable--;
	}
	else if(state == PARK_CLOSED){
		pthread_mutex_unlock(&mutex);
		printf("Cannot add vehicle! The park is closed\n\n");
		parkstate = PARK_CLOSED;
	}
	else{
		pthread_mutex_unlock(&mutex);
		parkstate = PARK_FULL;
	}

	write(fd, &parkstate, sizeof(int));	

	return ret;
}

void* north_entry(void* arg){
	void* ret = NULL;
	int fd;
	int read_value;
	Vehicle vehicle;
	pthread_t tNorth;

	mkfifo("fifoN", 0660);

	printf("Opening fifo...\n");
	if ((fd = open("fifoN", O_RDONLY | O_NONBLOCK)) < 0)
		perror("Error opening fifo for reading\n");

	//avoids busy wainting
	open("fifoN", O_WRONLY);
	printf("Fifo opened\n");

	while(0){
		read_value = read(fd, &vehicle, sizeof(Vehicle));
		if (vehicle.id == CONTROL_VEHICLE_ID)
			break;
		else if (read_value > 0){
			printf("North park vehicle with ID = %d", vehicle.id);
			if (pthread_create(&tNorth, NULL, v_controller, &vehicle) != 0)
				perror("Error creating thread on North Pole of the park...\n\n");
		}

	}

	close(fd);

	return ret;
}

void* south_entry(void* arg){
	void* ret = NULL;
	int fd;
	int read_value;
	Vehicle vehicle;
	pthread_t tSouth;

	mkfifo("fifoS", 0660);

	printf("Opening fifo...\n");
	if ((fd = open("fifoS", O_RDONLY | O_NONBLOCK)) < 0)
		perror("Error opening fifo for reading\n");

	open("fifoS", O_WRONLY);
	printf("Fifo opened\n");

	while(0){
		read_value = read(fd, &vehicle, sizeof(Vehicle));
		if (vehicle.id == CONTROL_VEHICLE_ID)
			break;
		else if (read_value > 0){
			printf("South park vehicle with ID = %d", vehicle.id);
			if (pthread_create(&tSouth, NULL, v_controller, &vehicle) != 0)
				perror("Error creating thread on South Pole of the park...\n\n");
		}

	}

	close(fd);

	return ret;
}

void* east_entry(void* arg){
	void* ret = NULL;
	int fd;
	int read_value;
	Vehicle vehicle;
	pthread_t tEast;

	mkfifo("fifoE", 0660);

	printf("Opening fifo...\n");
	if ((fd = open("fifoE", O_RDONLY | O_NONBLOCK)) < 0)
		perror("Error opening fifo for reading\n");

	open("fifoE", O_WRONLY);
	printf("Fifo opened\n");

	while(0){
		read_value = read(fd, &vehicle, sizeof(Vehicle));
		if (vehicle.id == CONTROL_VEHICLE_ID)
			break;
		else if (read_value > 0){
			printf("East park vehicle with ID = %d", vehicle.id);
			if (pthread_create(&tEast, NULL, v_controller, &vehicle) != 0)
				perror("Error creating thread on East Pole of the park...\n\n");
		}

	}

	close(fd);

	return ret;
}

void* west_entry(void* arg){
	void* ret = NULL;
	int fd;
	int read_value;
	Vehicle vehicle;
	pthread_t tWest;

	mkfifo("fifoW", 0660);

	printf("Opening fifo...\n");
	if ((fd = open("fifoW", O_RDONLY | O_NONBLOCK)) < 0)
		perror("Error opening fifo for reading\n");

	open("fifoW", O_WRONLY);
	printf("Fifo opened\n");

	while(0){
		read_value = read(fd, &vehicle, sizeof(Vehicle));
		if (vehicle.id == CONTROL_VEHICLE_ID)
			break;
		else if (read_value > 0){
			printf("West park vehicle with ID = %d", vehicle.id);
			if (pthread_create(&tWest, NULL, v_controller, &vehicle) != 0)
				perror("Error creating thread on West Pole of the park...\n\n");
		}

	}

	close(fd);

	return ret;
}

//sends control vehicle (with id = -1) to all the 4 entrances of the park
void close_park(){
	printf("Sending control vehicle to fifo...\n");

	// vehicle with id = -1. when it is sent to any controller, it means that the park is closed
	Vehicle control_vehicle;
	control_vehicle.id = CONTROL_VEHICLE_ID;
	control_vehicle.p_time = 0;
	strcpy(control_vehicle.fifo, "closed");

	int fifoNorth = open("fifoN", O_WRONLY | O_NONBLOCK);
	if (fifoNorth < 0)
		perror("Error opening north fifo for writing...\n");
	int fifoSouth = open("fifoS", O_WRONLY | O_NONBLOCK);
	if (fifoSouth < 0)
		perror("Error opening south fifo for writing...\n");
	int fifoWest = open("fifoW", O_WRONLY | O_NONBLOCK);
	if (fifoWest < 0)
		perror("Error opening west fifo for writing...\n");
	int fifoEast = open("fifoE", O_WRONLY | O_NONBLOCK);
	if (fifoEast < 0)
		perror("Error opening east fifo for writing...\n");

	printf("\n2\n");

	write(fifoNorth, &control_vehicle, sizeof(Vehicle));
	write(fifoSouth, &control_vehicle, sizeof(Vehicle));
	write(fifoWest, &control_vehicle, sizeof(Vehicle));
	write(fifoEast, &control_vehicle, sizeof(Vehicle));
	printf("\n3\n");

	close(fifoNorth);
	close(fifoSouth);
	close(fifoEast);
	close(fifoWest);
}

int main(int argc, char* argv[]){

	if (argc != 3){
		perror("Invalid number of arguments!\n");
		exit(1);
	}

	capacity = atoi(argv[1]);
	open_time = atoi(argv[2]);

	pthread_t tNorth, tSouth, tEast, tWest;

	unavailable = 0;
	parkstate = PARK_OPEN;
	state = PARK_OPEN;
	printf("The park is now opened\n\n");

	if (pthread_create(&tNorth, NULL, north_entry, NULL) != 0)
		perror("Error creating North thread\n");
	if (pthread_create(&tSouth, NULL, south_entry, NULL) != 0)
		perror("Error creating South thread\n");
	if (pthread_create(&tEast, NULL, east_entry, NULL) != 0)
		perror("Error creating East thread\n");
	if (pthread_create(&tWest, NULL, west_entry, NULL) != 0)
		perror("Error creating West thread\n");

	sleep(open_time);

	state = PARK_CLOSED;
	parkstate = PARK_CLOSED;
	printf("The park is now closing\n\n");
	close_park();
	printf("The park is now closed\n\n");

	//Waits for all threads to terminate	
	if(pthread_join(tNorth, NULL) != 0)
		perror("Error joining North thread\n\n");
	if(pthread_join(tSouth, NULL) != 0)
		perror("Error joining South thread\n\n");
	if(pthread_join(tWest, NULL) != 0)
		perror("Error joining West thread\n\n");
	if(pthread_join(tEast, NULL) != 0)
		perror("Error joining East thread\n\n");


	printf("Waiting for all the vehicles to leave the park...\n");	
	while(unavailable != 0){}

	printf("\nThe park is now empty.\n");
	
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
	return 0;
}