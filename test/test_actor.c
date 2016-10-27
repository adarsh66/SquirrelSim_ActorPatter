/* Code to test out the Actor functions (actor.c). Compile it and execute
   This test will also cover basic send and recv messages
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "mpi.h"
#include "../src/actor.h"
#include "../src/message.h"
#include "../processpool/pool.h"

#define BUFFERSIZE 1000

enum COMMAND_TYPES {
	SQUIRREL_VISIT,
	POPULATION_REQUEST,
	INITIATE_ACTOR
};

int main(int argc, char* argv[]) 
{
	//Initialize MPI
	MPI_Init(&argc, &argv);
	char buff[BUFFERSIZE];
	MPI_Buffer_attach(buff, BUFFERSIZE);
	int statusCode = processPoolInit();
	int myRank, response, rstatus;
	message_package newMessage;	
	if (statusCode == 1) {
		// A worker so do the worker tasks
		int parentId;
		parentId = getCommandData();
		MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
		initActor(myRank, MPI_COMM_WORLD);
		if (parentId==0) { msg_send(parentId, SQUIRREL_VISIT, 101+myRank*3); }
		rstatus = pending_recv(parentId);
		while (rstatus ==0) {
			rstatus = pending_recv(parentId);
		}
		msg_recv(&newMessage, parentId);
		printf("Message Received; MyRank=%d\tSource=%d\tMsgType=%d\tMsgData=%d\n", 
			getMsgDestination(newMessage), getMsgSource(newMessage), getMsgType(newMessage), getMsgData(newMessage));
		
		//give birth to new actors if you are first gen
		if (parentId == 0) {
			int childId = actor_birth();
			msg_send(childId, INITIATE_ACTOR, 404+myRank*3);
			
		} else {
			shutdownPool();
		}
		actor_die();
		printf("Is it dead=%d\n", get_actor_status());

	} else if (statusCode == 2) {
		//master - rank 0
		myRank = 0;
		initActor(myRank, MPI_COMM_WORLD);
		int workerPid = startWorkerProcess();
		msg_send(workerPid, POPULATION_REQUEST, 202.2);
		rstatus = pending_recv(workerPid);
		while (rstatus ==0) {
			rstatus = pending_recv(workerPid);
		}
		msg_recv(&newMessage, workerPid);
		printf("Message Received; MyRank=%d\tSource=%d\tMsgType=%d\tMsgData=%d\n", 
			getMsgDestination(newMessage), getMsgSource(newMessage), getMsgType(newMessage), getMsgData(newMessage));
		
		int masterStatus = masterPoll();
		while (masterStatus) {
			masterStatus=masterPoll();
		}
	}
	printf("I am here %d\n", myRank);
	// Finalizes the process pool, call this before closing down MPI
	processPoolFinalise();
	// Finalize MPI, ensure you have closed the process pool first
	MPI_Finalize();
	return 0;
}

