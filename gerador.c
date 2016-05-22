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


float gen_vehicle(){
	Vehicle new_vehicle;
	new_vehicle.id = id;
	id++;
	int r = rand() % 4;

	pthread_t genV;

	switch(r){
		case 0:
			new_vehicle.dir = NORTH;
			new_vehicle.fifo = "fifoN";
			break;
		case 1:
			new_vehicle.dir = SOUTH;
			new_vehicle.fifo = "fifoS";
			break;
		case 2:
			new_vehicle.dir = EAST;
			new_vehicle.fifo = "fifoE";
			break;
		case 3:
			new_vehicle.dir = WEST;
			new_vehicle.fifo = "fifoW";
			break;
	}

	float park_time = (rand() % 10) + 1 * clock_ticks;
	new_vehicle.p_time = park_time; 

	if(pthread_create(&genV, NULL, process_V, &vehicle) != 0)
		perror("Error creating process_v thread...\n");

	int x = rand() % 10;
	int tick_till_ncar;
	//20 %
	if(r < 2){
		tick_till_ncar = 2;
	}
	//30 %
	else if(r < 5){
		tick_till_ncar = 1;
	}	
	//50 %
	else
		tick_till_ncar = 0;

	return tick_till_ncar;

}

int main(int argc, char* argv[]){

	if (argc != 3){
		perror("Wrong number of arguments")
		exit(1);s
	}

	srand(time(NULL));

	gen_time = (float) atoi(argv[1]);
	clock_ticks = (float) atoi(argv[2]);



}