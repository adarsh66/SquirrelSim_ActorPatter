
MF=	Makefile

#Select DEBUGLOG if you do want to see debug msgs (very granular data)
#debug= -DDEBUGLOG
debug=
#Select INFOLOG if you want to see Info msgs in the output (high level sys info)
#infolog= -DINFOLOG
infolog=

#morar
CC=	mpicc
DEBUG ?= 1
CFLAGS = -lm -fastsse $(debug) $(infolog)

#personal laptop
#CC=	cc
#CFLAGS= -g

LFLAGS=	-lm

EXE	=	squirrel_disease_sim

SRC= \
	main.c \
	src/message.c \
	src/actor.c \
	src/squirrel-functions.c \
	src/ran2.c \
	processpool/pool.c \
	squirrelsim/runsqsim.c \
	squirrelsim/simclock.c \
	squirrelsim/squirrel.c \
	squirrelsim/landcell.c 

INC=\
	src/message.h \
	src/actor.h \
	src/squirrel-functions.h \
	src/ran2.h \
	src/log.h \
	processpool/pool.h \
	squirrelsim/runsqsim.h \
	squirrelsim/simclock.h \
	squirrelsim/squirrel.h \
	squirrelsim/landcell.h \
	squirrelsim/sim_params.h \
	squirrelsim/message_commands.h
#
# No need to edit below this line
#


.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -o $(<:.c=.o) -c $<

all:	$(EXE)

$(EXE):	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LFLAGS)

$(OBJ):	$(INC)

$(OBJ):	$(MF)

clean:
	rm -f $(OBJ) $(EXE) core
