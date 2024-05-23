all: src/main.o src/main_par.o
src/main.o: src/main.cpp src/utils.hxx src/canvas.hxx
	mpic++ --std=c++20 src/utils.hxx src/canvas.hxx src/main.cpp -o src/main.o
src/main_par.o: src/main_par.cpp src/utils.hxx src/canvas.hxx
	mpic++ --std=c++20 src/utils.hxx src/canvas.hxx src/main_par.cpp -o src/main_par.o
