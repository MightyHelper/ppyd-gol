src/main.o: src/main.cpp src/utils.hxx src/canvas.hxx
	mpic++ --std=c++20 -O5 src/utils.hxx src/canvas.hxx src/main.cpp -o src/main.o
