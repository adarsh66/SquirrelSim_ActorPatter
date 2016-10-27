/* Base class for the message_package used for communication
 * Message package contains integer data and associated metadata
 * Metadata: command_type, source, destination
 * Also contains get methods to source metadata from object
*/

#include <stdio.h>
#include <assert.h>
#include "message.h"
#include "log.h"

message_package createMsgPackage(char, int, int, int);
int getMsgData(message_package);
char getMsgType(message_package);
int getMsgSource(message_package);

/* returns the standadized version of the message package 
   which will be communicated between the different systems
*/
message_package createMsgPackage(char command_type, int data, int source, int destination)
{
	message_package msg;
	msg.command_type = command_type;
	msg.data = data;
	msg.source = source;
	msg.destination = destination;
	return msg;
}

/* Following are attribute access functions */
int getMsgData(message_package msg)
{
	return msg.data;
}

char getMsgType(message_package msg)
{
	return msg.command_type;
}

int getMsgSource(message_package msg)
{
	return msg.source;
}

int getMsgDestination(message_package msg)
{
	return msg.destination;
}

/* Prints out the message details to std err when compiled with DEBUG flag  ON
*/
void printMsg(message_package msg)
{
	debug("---MESSAGE DETAILS: Source=%d, Destination=%d, data=%d, command=%c --\n", 
			msg.source, msg.destination, msg.data, msg.command_type);
}

/* Destructor for this class */
void msg_destroy(message_package *msg)
{
	assert(msg !=NULL);

	free(msg->command_type);
	free(msg->data);
	free(msg->source);
	free(msg->destination);

	free(msg);
}