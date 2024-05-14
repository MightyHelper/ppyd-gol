src/main.o: src/main.cpp
	mpic++ --std=c++20 src/utils.hxx src/canvas.hxx src/main.cpp -o src/main.o
