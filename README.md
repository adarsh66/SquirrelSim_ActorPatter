Squirrel Disease Spread Simulation implemented in Parallel using Actor Pattern (MPI)
====================================================================================
* Exam ID: B077962
* Module: Parallel Design Patterns (Univ. of Edinburgh (EPCC))

Actor model
-----------
This is the implementation of the Squirrel disease spread simulation in the Actor pattern using MPI. It has been implemented in C. 

Build and run
--------------
You can build this by running the Makefile provided here. You need to have MPI environment setup, preferablly mpicc. You can do this by `module load PrgEnv-pgi` if you have that facility

`make`

If you wish to turn on the different levels of logging available, open the Makefile and edit the following options. By default they are turned off. 

```bash
#Select DEBUGLOG if you do want to see debug msgs (very granular data)
#debug= -DDEBUGLOG
debug=
#Select INFOLOG if you want to see Info msgs in the output (high level sys info)
#infolog= -DINFOLOG
infolog=
```

To run this on local machine using MPI

```bash
mpiexec -np 64 ./squirrel_disease_sim 
```

Run on morar
------------
To run on morar, please run the following command (change 64 to number of processors you wish to run on)

```bash
qsub -pe mpi 64 squirrel_disease_sim.sge
```

Folders and contents
--------------------
* /processpool - contains the code for the process pool used for this framework. It was provided as part of this assignment
* /src - contains the code for the framework implementation as well as the code provided by the biologist for the simulation
	- /src/actor.c - Actor in the framework
	- /src/message_package.c - Message package used for communication
	- /src/run2.c and /src/squirrel-functions.c - Biologist's functions for this simulation
* /squirrelsim - contains the code base for the Squirrel disease simulation
	- /squirrelsim/squirrel.c - code for the Squirrel actor
	- /squirrelsim/landcell.c - code for the Landcell actor
	- /squirrelsim/simclock.c - code for the Clock actor
	- /squirrelsim/runsqsim.c - code for the simulation runner
	- /squirrelsim/message_commands.h - the command types which can be issued in this simulation under framework
	- /squirrelsim/sim_params.h - the initialisation parameters for this simulation