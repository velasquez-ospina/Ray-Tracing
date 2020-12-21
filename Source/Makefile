
output: main.o
	mpirun -np 4 a.out

main.o: main.cpp
	mpic++ main.cpp json11.cpp

clean:
	rm a.out Scene.bmp
