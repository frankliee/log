main: main.o log_manager.o tran_manager.o lock_free.o
	g++ -o main main.o log_manager.o  tran_manager.o -std=c++11 -O0 -g \
	-lcaf_core -lcaf_io -latomic
 

main.o: main.cpp log_manager.o tran_manager.o lock_free.o 
	g++ main.cpp -std=c++11 -O0 -g -c  

log_manager.o: log_manager.cpp log_manager.hpp lock_free.o
	g++ log_manager.cpp -std=c++11 -O0 -g -c  

tran_manager.o: tran_manager.cpp tran_manager.hpp log_manager.o lock_free.o
	g++ tran_manager.cpp -std=c++11 -O0 -g -c  

lock_free.o: lock_free.cpp lock_free.hpp
	g++ lock_free.cpp -std=c++11 -O0 -g -c

clean:
	rm main.o log_manager.o tran_manager.o lock_free.o