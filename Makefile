CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
SRCFILES = main.cpp server.cpp reply.cpp config.cc config_parser.cc

webserver:
	$(CXX) -o $@ $^ $(CXXFLAGS) $(SRCFILES) -lboost_system 

clean:
	rm -rf *.o webserver
