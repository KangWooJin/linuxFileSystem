FileSystem : main.o Command.h Directory.h FileSystem.h File.h Value.h  
	g++ -o FileSystem main.o 
main.o : main.cpp Command.h Directory.h FileSystem.h File.h Value.h  
	g++ -c main.cpp
clean :
	rm -f FileSystem main.o 

