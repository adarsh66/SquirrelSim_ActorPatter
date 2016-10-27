#ifndef MESSAGE_COMMANDS_
#define MESSAGE_COMMANDS_

/*	The list below is the list of possible
	commands which can be passed in this simulation using the framework
	For other simulations, you can create new command
	lists based on requirements.
*/

enum message_commands {
	//for initialising the coordinates for each actor
	INITIALISE_LAND,
	INITIALISE_SQUIRREL,
	INITIALISE_CLOCK,
	//for indicating a squirrel has landed on the cell
	SQUIRREL_VISIT,
	//for sending population influx from land cells to others
	POPULATION_INFLUX,
	//for sending infection level from land cells to others
	INFECTION_LEVEL,
	//for sending squirrel's birth or death status to clock
	SQUIRREL_STATUS,
	//for sending clock ID to the squirrels
	CLOCK_INFO,
	//for sending clock updates to land cells
	CLOCK_MONTH,
	//for letting the actors know sim is going to end. clock to others
	SIMULATION_END
};

#endif
