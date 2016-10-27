#ifndef SIM_PARAMS_H_
#define SIM_PARAMS_H_

/*	Simulation Parameters 
 *	Hardcoded here for now, will look into making it configurable
 *	This is the only information which will be shared commonly across all Actors
*/

//Total number of grid cells. Doesn't matter if it is a regular grid or not
#define LANDCELLS 16
//Total number of squirrels to begin with
#define TOT_SQUIRRELS 34
//Number of those squirrels which will be infected at birth
#define INF_SQUIRRELS 4
// 24 months total simulation time
#define NUM_MONTHS 24 
//in seconds, length of one month
#define MON_LENGTH 0.01
//Window size for squirrel average calculations
#define WINDOWSIZE 50 

#endif
