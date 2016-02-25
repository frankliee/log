main: main.o log_manager.o
	g++ -o main main.o log_manager.o -std=c++11 -O0 -g -lcaf_core -lcaf_io

main.o: main.cpp log_manager.hpp
	g++ main.cpp -std=c++11 -O0 -g -c

log_manager.o: log_manager.cpp log_manager.hpp
	g++ log_manager.cpp -std=c++11 -O0 -g -c
	
clean:
	rm main.o log_manager.o