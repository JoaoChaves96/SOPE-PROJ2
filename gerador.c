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

float gen_time;
float clock_ticks;
int id = 0;

typedef enum {NORTH, SOUTH, EAST, WEST} Direction;

typedef struct{
	int id;
	float p_time;
	char fifo[FIFO_LENGTH];
	Direction dir;
} Vehicle;


void* process_V(void* arg){
	void* ret = NULL;
	Vehicle vehicle = *(Vehicle*) arg;

	int fdWrite;

	switch(vehicle.dir){
		case NORTH:
			fdWrite = open("fifoN", O_WRONLY | O_NONBLOCK);
			break;
		case SOUTH:
			fdWrite = open("fifoS", O_WRONLY | O_NONBLOCK);
			break;
		case WEST:
			fdWrite = open("fifoW", O_WRONLY | O_NONBLOCK);
			break;
		case EAST:
			fdWrite = open("fifoE", O_WRONLY | O_NONBLOCK);
			break;
	}

	mkfifo(vehicle.fifo, 0660);

	if (fdWrite != -1){
		write(fdWrite, &vehicle, sizeof(Vehicle));
		printf("Vehicle%d added\n", vehicle.id);
		close(fdWrite);
	}


	unlink(vehicle.fifo);

	return ret;
}

float gen_vehicle(){
	Vehicle *new_vehicle = (Vehicle*)malloc(sizeof(Vehicle));;
	new_vehicle->id = id;
	id++;
	int r = rand() % 4;

	pthread_t genV;

	switch(r){
		case 0:
			new_vehicle->dir = NORTH;
			break;
		case 1:
			new_vehicle->dir = SOUTH;
			break;
		case 2:
			new_vehicle->dir = EAST;
			break;
		case 3:
			new_vehicle->dir = WEST;
			break;
	}

	sprintf(new_vehicle->fifo, "%s%d", "fifo", new_vehicle->id);

	float park_time = (rand() % 10) + 1 * clock_ticks;
	new_vehicle->p_time = park_time; 

	if(pthread_create(&genV, NULL, process_V, new_vehicle) != 0)
		perror("Error creating process_v thread...\n");

	int x = rand() % 10;
	int tick_till_ncar;
	//20 %
	if(x < 2){
		tick_till_ncar = 2;
	}
	//30 %
	else if(x < 5){
		tick_till_ncar = 1;
	}	
	//50 %
	else
		tick_till_ncar = 0;

	return tick_till_ncar;

}

int main(int argc, char* argv[]){

	if (argc != 3){
		perror("Wrong number of arguments");
		exit(1);
	}

	srand(time(NULL));

	gen_time = (float) atoi(argv[1]);
	clock_ticks = (float) atoi(argv[2]);

	float tot_ticks;
	int ticks_for_ncar = 0;

	tot_ticks = (gen_time/clock_ticks) * 1000; //number of events

	do{
		if (ticks_for_ncar == 0)
			ticks_for_ncar = gen_vehicle(tot_ticks);
		else
			ticks_for_ncar--;
		usleep(clock_ticks * 1000);
		tot_ticks--;
	} while (tot_ticks > 0);

	pthread_exit(NULL);
}