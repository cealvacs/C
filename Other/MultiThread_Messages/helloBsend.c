/*
NAME:       Carlos Alva
Section:    COP4338-U01

I affirm that this program is entirely my own work and none of it is the work
of any other person.

CARLOS ALVA
*/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define  MASTER		0

int main (int argc, char *argv[])
{
int   numtasks, taskid, len;
char hostname[MPI_MAX_PROCESSOR_NAME];
int partner;
int recv;
MPI_Status status;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);

//Finds Partner
if(taskid < numtasks/2)
{
	partner = numtasks/2 + taskid;
	//printf("Task %d is partner with %d\n", taskid, partner);
}
else if (taskid >= numtasks/2)
{
	partner = taskid - numtasks/2;
	//printf("Task %d is partner with %d\n", taskid, partner);
}

//Send and Receive, save received data in recv variable
if(numtasks > 1)
{
	MPI_Send(&taskid, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);	
	MPI_Recv(&recv, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);

}
//Prints current process taskid and recv message
printf("Task:%d is partner with Task: %d\n\n", taskid, recv);

//Only process 0 prints
if (taskid == MASTER)
{
   printf("\nCarlos Alva\nProcess has %d tasks\n", numtasks);
}
MPI_Finalize();
return 0;
}

