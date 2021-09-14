TARGET = example svr cli
CXX = g++
CFLAGS += -g -O2 -Wall -I../Include -L../Lib -lNetFramework64 -lInfra64 -lpthread -lc -lrt

all : $(TARGET)

example : example.o
	$(CXX) -o example example.o $(CFLAGS)

example.o : example.cpp
	$(CXX) -c $(CFLAGS) example.cpp 

svr : NetSvr.o
	$(CXX) -o svr NetSvr.o $(CFLAGS)

NetSvr.o : NetSvr.cpp
	$(CXX) -c $(CFLAGS) NetSvr.cpp 

cli : NetCli.o
	$(CXX) -o cli NetCli.o $(CFLAGS)

NetCli.o : NetCli.cpp
	$(CXX) -c $(CFLAGS) NetCli.cpp 

clean:
	rm -f *.o
	rm -f $(TARGET)

