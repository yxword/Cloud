TARGET = svr cli
CXX = g++
CFLAGS += -g -O2 -Wall -I../Include -L../Lib -lNetFramework64 -lInfra64 -lpthread -lc -lrt

all : $(TARGET)

StringBuffer.o : StringBuffer.cpp
	$(CXX) -c $(CFLAGS) StringBuffer.cpp 

svr : FileServer.o ServerMain.o NetSession.o StringBuffer.o
	$(CXX) -o svr FileServer.o ServerMain.o NetSession.o StringBuffer.o $(CFLAGS)

NetSession.o : NetSession.cpp
	$(CXX) -c $(CFLAGS) NetSession.cpp 

FileServer.o : FileServer.cpp
	$(CXX) -c $(CFLAGS) FileServer.cpp 

ServerMain.o : ServerMain.cpp
	$(CXX) -c $(CFLAGS) ServerMain.cpp 
	

cli : FileClient.o ClientMain.o StringBuffer.o
	$(CXX) -o cli FileClient.o ClientMain.o StringBuffer.o $(CFLAGS)

FileClient.o : FileClient.cpp
	$(CXX) -c $(CFLAGS) FileClient.cpp 	

ClientMain.o : ClientMain.cpp
	$(CXX) -c $(CFLAGS) ClientMain.cpp 


clean:
	rm -f *.o
	rm -f $(TARGET)

