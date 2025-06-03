CXX=g++
CXXFLAGS=-std=c++11 -pthread -I./raknet -I./raknet/SAMP

RAKNET_SRC=$(wildcard raknet/*.cpp)
CLIENT_SRC=client/console_client.cpp

all: console_client

console_client: $(RAKNET_SRC) $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(RAKNET_SRC) $(CLIENT_SRC) -o console_client

clean:
	rm -f console_client
