#ifndef ACTOR_H_
#define ACTOR_H_
#include "message.h"

//Initialize the actor with process id, status (live=1 or dead=0), and MPI_Comm used
typedef struct actor {
	int id;
	int status;
	MPI_Comm comm;
} actor;

//initialize the Actor 
void initActor(int, MPI_Comm);
//give birth to a new actor
int actor_birth();
//kill this process
int actor_die();
//get actor id
int get_actor_id();
//get actor status
int get_actor_status();
//get actor Communicator
MPI_Comm get_actor_comm();
//sends a message of type message_package. takes in integer data type input
void msg_send(int, char, int);
//checks if there are any pending receives in the queue for this process
int pending_recv(int);
//issues a MPI_Recv to receive msg from a given source id.
int msg_recv(message_package*, int);
//Print out the state of the actor to std out
void print_actor_state();
//Checks if there is a global comm to stop all the workers 
int should_actor_stop();


#endif
