# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL -lassimp `Magick++-config --cppflags --cxxflags --ldflags --libs`

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/Model

../bin/Model: ../src/main.cpp shader.o mesh.o planet.o
	$(CC) $(CXXFLAGS) ../src/main.cpp shader.o mesh.o planet.o -o ../bin/Model $(LIBS)

shader.o: ../src/shader.cpp ../src/shader.h
	$(CC) $(CXXFLAGS) -c ../src/shader.cpp ../src/shader.h $(LIBS)

mesh.o: ../src/mesh.cpp ../src/mesh.h
	$(CC) $(CXXFLAGS) -c ../src/mesh.cpp ../src/mesh.h $(LIBS)

planet.o: ../src/planet.cpp ../src/planet.h
	$(CC) $(CXXFLAGS) -c ../src/planet.cpp ../src/planet.h $(LIBS)	

clean:
	$(RM) ../bin/Model
	$(RM) *.~ *.o
	$(RM) ../src/*.gch*
