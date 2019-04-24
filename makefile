Assign7: main.o
	g++ -Wall -o Assign7 main.o

main.o: main.cpp
	g++ -Wall -c -std=c++17 main.cpp

clean:
	-rm *.o Assign7
