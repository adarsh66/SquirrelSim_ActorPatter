#ifndef SQUIRREL_H_
#define SQUIRREL_H_

//Main function
void squirrel_main(int);

//Internal functions
void initSquirrel(int);
int get_infection_status();
float get_avg(int*, int);
void squirrel_death();
void squirrel_birth();
void squirrel_infection();

#endif
