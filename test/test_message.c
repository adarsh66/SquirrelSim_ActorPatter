#include <stdio.h>
#include "../message.h"

/* Test cases for Message Package Functions */

enum COMMAND_TYPES {
	SQUIRREL_VISIT,
	POPULATION_REQUEST,
	INITIATE_ACTOR
};

int test_msgsource(int source, int data, enum COMMAND_TYPES command);
int test_msgdata(int source, int data, enum COMMAND_TYPES command);
int test_msgtype(int source, int data, enum COMMAND_TYPES command);


int main(int argc, char *argv)
{
	int status[3];
	int data = 10;
	int source = 0;
	int i;
	status[0] = test_msgtype(source, data, SQUIRREL_VISIT);
	status[1] = test_msgdata(source, data, POPULATION_REQUEST);
	status[2] = test_msgsource(source, data, INITIATE_ACTOR);
	
	for (i=0;i<3;i++) {
		printf("Status[%d] = %d\n", i, status[i]);
	}
	return 0;

}

int test_msgtype(int source, int data, enum COMMAND_TYPES command)
{	
	message_package newMessage = createMsgPackage(command, data, source);
	if (newMessage.command_type == command) { return 1; }
	else { return 0; }
}

int test_msgdata(int source, int data, enum COMMAND_TYPES command)
{	
	message_package newMessage = createMsgPackage(command, data, source);
	if (newMessage.data == data) { return 1; }
	else { return 0; }
}

int test_msgsource(int source, int data, enum COMMAND_TYPES command)
{
	message_package newMessage = createMsgPackage(command, data, source);
	if (newMessage.source == source) { return 1; }
	else { return 0; }
}


