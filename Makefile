CC      = gcc
CXX     = g++
LINK    = g++
CFLAGS  = -g -lOsiClp -lCbc -lClp
BIN		= ./bin/
RM		= rm -rf
MKDIR	= mkdir



SRCS	= $(wildcard ./src/*.cpp)
SRCS	+= ./ILP/ILP.cpp 
DIRS	= bin obj

CXX_OBJS	= obj
CXX_BINS	= bin

all: $(DIRS) codegen simulate

$(DIRS):
	$(MKDIR) $@

codegen:
	$(CXX) main.cpp $(SRCS) $(CFLAGS) -o $(CXX_BINS)/codegen


simulate:
	$(CXX) simulation/simulate.cpp $(SRCS) $(CFLAGS) -o $(CXX_BINS)/simulate
	cp $(CXX_BINS)/simulate simulation/simulate


.PHONY:clean
clean:
	$(RM) $(CXX_BINS) $(CXX_OBJS) simulation/simulate

