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
#define LEAVING_VEHICLE 5


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

float gen_time;
float clock_ticks;
int id = 0;
int ger_fd;
int tick = 0;

typedef enum {NORTH, SOUTH, EAST, WEST} Direction;

typedef struct{
	int id;
	float p_time;
	char fifo[FIFO_LENGTH];
	float initTick;
	int ticks;
	Direction dir;
} Vehicle;

void write_log(Vehicle *vehicle, int state){
	char dest[2];
	char line[200];
	char obs[10];

	switch(state){
		case PARKING_VEHICLE:
			strcpy(obs, "entrada");
			break;
		case LEAVING_VEHICLE:
			strcpy(obs, "saida");
			break;
		case PARK_FULL:
			strcpy(obs, "cheio");
			break;
		case PARK_CLOSED:
			strcpy(obs, "fechado");
			break;
	}

	switch(vehicle->dir){
		case NORTH:
			strcpy(dest, "N");
			break;
		case SOUTH:
			strcpy(dest, "S");
			break;
		case EAST:
			strcpy(dest, "E");
			break;
		case WEST:
			strcpy(dest, "W");
			break;
	}

	if (state == LEAVING_VEHICLE)
		sprintf(line, "%8d ; %7d ; %6s ; %10d ; %6d ; %7s\n", (int)vehicle->initTick+vehicle->ticks, vehicle->id, dest, (int) vehicle->ticks, (int)(clock_ticks-vehicle->initTick), obs);
	else
		sprintf(line, "%8d ; %7d ; %6s ; %10d ; %6s ; %7s\n", (int)vehicle->initTick, vehicle->id, dest, (int) vehicle->ticks, "?", obs);

	write(ger_fd, &line, strlen(line));
	//printf("Escreveu no log\n");

	strcpy(line, "");

}

void* process_V(void* arg){
	void* ret = NULL;
	Vehicle vehicle = *(Vehicle*) arg;

	int fdWrite, fdRead;
	int state;

	if (mkfifo(vehicle.fifo, 0660) != 0)
		perror("Error making fifo\n");

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
		default:
			break;
	}


	if (fdWrite != -1){
		write(fdWrite, &vehicle, sizeof(Vehicle));
		//printf("Vehicle%d added\n", vehicle.id);
		close(fdWrite);

		fdRead = open(vehicle.fifo, O_RDONLY);
		if (fdRead != -1){
			read(fdRead, &state, sizeof(int));


			//pthread_mutex_lock(&mutex);
			write_log(&vehicle, state);
			//pthread_mutex_unlock(&mutex);
			if(state != PARK_FULL){
				read(fdRead, &state, sizeof(int));
				state = LEAVING_VEHICLE;
				//printf("%d\n", state);
				//state = LEAVING_VEHICLE;
				//close(fdRead);

			}

		}
		else; //park is closed
	}
	else
		state = PARK_FULL;

	//pthread_mutex_lock(&mutex);
	write_log(&vehicle, state);
	//pthread_mutex_unlock(&mutex);


	/*if (state == PARKING_VEHICLE){
		state = LEAVING_VEHICLE;
	}*/

	//close(fdWrite);
	//close(fdRead);
	unlink(vehicle.fifo);

	return ret;
}

float gen_vehicle(float tot, float curr){
	Vehicle *new_vehicle = (Vehicle*)malloc(sizeof(Vehicle));
	//Vehicle new_vehicle;
	new_vehicle->id = id;
	id++;
	int r = rand() % 4;

	new_vehicle->initTick = curr;

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

	char buff[100];
	sprintf(buff, "%s%d", "fifo", new_vehicle->id);

	strcpy(new_vehicle->fifo, buff);

	float park_time = ((rand() % 10) + 1) * clock_ticks;
	new_vehicle->p_time = park_time; 
	new_vehicle->ticks = park_time/clock_ticks;

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

	ger_fd = open("gerador.log", O_WRONLY | O_CREAT, 0660);
	char ger_format[] = "Registo do gerador:\nt(ticks) ; id_viat ; destin ; t_estacion ; t_vida ; observ\n";

	pthread_mutex_lock(&mutex);
	write(ger_fd, ger_format, strlen(ger_format));
	pthread_mutex_unlock(&mutex);

	srand(time(NULL));

	gen_time = (float) atoi(argv[1]);
	clock_ticks = (float) atoi(argv[2]);

	float num_ticks;
	int ticks_for_ncar = 0;

	num_ticks = (gen_time/clock_ticks) * 1000; //number of events
	do{
		if (ticks_for_ncar == 0)
			ticks_for_ncar = gen_vehicle(clock_ticks, tick);
		else
			ticks_for_ncar--;
		usleep(clock_ticks * 1000);
		tick++;
	} while (num_ticks != tick);

	close(ger_fd);

	pthread_mutex_destroy(&mutex);

	pthread_exit(NULL);
}