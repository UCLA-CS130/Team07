CXX=g++
CXXOPTIMIZE= -O2

CXXFLAGS= -g -Wall -std=c++11 #-static-libgcc -static-libstdc++ -pthread -Wl,--no-as-needed -std=c++11 
#LDFLAGS= -lboost_system -lboost_regex -lcrypto -lssl
LDFLAGS= -static-libgcc -static-libstdc++ -pthread -Wl,-Bstatic -lssl -lcrypto -lboost_thread -lboost_filesystem -lboost_system -lboost_regex -Wl,-Bdynamic -ldl
SRCFILES = server.cpp response.cpp request.cpp request_parser.cpp config.cc config_parser.cc request_handler.cpp echo_handler.cpp file_handler.cpp not_found_handler.cpp status_handler.cpp server_stats.cpp proxy_handler.cpp spaceteam_proxy_handler.cpp cpp-markdown/markdown.cpp cpp-markdown/markdown-tokens.cpp

GTEST_DIR = googletest/googtest
GMOCK_DIR = googletest/googlemock

all: CXXFLAGS += $(CXXOPTIMIZE)
all: SRCFILES += main.cpp
all: webserver

test: CXXFLAGS += -isystem ${GTEST_DIR}/include -isystem ${GMOCK_DIR}/include
test: SRCFILES += config_parser_test.cc config_test.cc server_test.cpp response_test.cpp file_handler_test.cpp request_parser_test.cpp echo_handler_test.cpp not_found_handler_test.cpp status_handler_test.cpp server_stats_test.cpp proxy_handler_test.cpp spaceteam_proxy_handler_test.cpp ${GTEST_DIR}/src/gtest_main.cc libgtest.a
test: | clean webserver_test

build_image:
	./build_image.sh

run_image:
	docker run --rm -t -p 8080:8080 httpserver

deploy:
	./deploy.sh

run:
	./webserver config

run_coverage:
	./webserver_test
	gcov -r server.cpp response.cpp config.cc config_parser.cc request_parser.cpp echo_handler.cpp file_handler.cpp request.cpp not_found_handler.cpp status_handler.cpp server_stats.cpp proxy_handler.cpp spaceteam_proxy_handler.cpp base_

webserver:
	$(CXX) -I /usr/include/boost/ -o $@ $^ $(CXXFLAGS) $(SRCFILES) $(LDFLAGS)

webserver_test: 
	$(CXX) $(CXXFLAGS) -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	$(CXX) -I /usr/include/boost/ -o $@ $^ $(CXXFLAGS) -fprofile-arcs -ftest-coverage $(SRCFILES) $(LDFLAGS)
#-I /usr/include/boost/ 

clean:
	rm -rf *.o *.gcov *.gcda *.gcno webserver webserver_test

integration:
	make clean && make
	python reverse_proxy_integration.py

integration_302:
	make clean && make
	python reverse_proxy_302_integration.py

spaceteam_proxy:
	make clean && make
	./proxy_handler_integration_test.sh

spaceteam_302:
	make clean && make
	./spaceteam_302_redirect_integration_test.sh

.PHONY: all clean deploy

