all: server

server: main.o db_manager.o ; c++ main.o db_manager.o -o server `pkg-config --libs --cflags libpqxx`

main.o: db_manager.o main.cpp ; c++ -std=c++11 -c main.cpp `pkg-config --libs --cflags libpqxx`

db_manager.o: db_manager.cpp ; c++ -std=c++11 -c db_manager.cpp `pkg-config --libs --cflags libpqxx`

clear: ; rm -rf *.o server
