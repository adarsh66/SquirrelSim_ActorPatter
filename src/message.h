#ifndef MESSAGE_H_
#define MESSAGE_H_

/* Communicate using struct based msgs which contain
   a command_type tag to indicate what type of msg this is
   integer data being communicated
   source will preserve the source rank id which sends the msg
*/
typedef struct message_package {
	char command_type;
	int data;
	int source;
	int destination;
} message_package;

//get a new message package which can be communicated
message_package createMsgPackage(char, int, int, int);
//Destructor
void msg_destroy(message_package *msg);
//message attribute access functions
int getMsgData(message_package);
char getMsgType(message_package);
int getMsgSource(message_package);
int getMsgDestination(message_package msg);

//print out the message details to std out
void printMsg(message_package msg);

#endif
