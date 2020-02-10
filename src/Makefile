
# files

EXE = shogi

OBJS = attack.o bit.o common.o eval.o sfen.o gen.o \
       hash.o libmy.o list.o main.o math.o move.o hand.o\
       pos.o score.o search.o sort.o thread.o tt.o util.o var.o \

# rules

all: $(EXE)

clean:
	$(RM) $(OBJS) .depend # keep exe

# general

CXX      = clang++ # g++ if you must, but beware of NPS!
CXXFLAGS = -pthread 
LDFLAGS  = -pthread 

# warnings

CXXFLAGS += -ansi -pedantic -Wall -Wextra

# C++

CXXFLAGS += -std=c++14 -fno-rtti

# optimisation

#CXXFLAGS += -O3 -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -DDEBUG
#LDFLAGS  += -O3 -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI

CXXFLAGS += -g -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -DDEBUG -fstandalone-debug
LDFLAGS  += -g -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI


CXXFLAGS += -flto
LDFLAGS  += -flto

# dependencies

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

.depend:
	$(CXX) $(CXXFLAGS) -MM $(OBJS:.o=.cpp) > $@

include .depend

