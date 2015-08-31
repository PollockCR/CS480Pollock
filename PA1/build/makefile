# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/Matrix

../bin/Matrix: ../src/main.cpp shader.o
	$(CC) $(CXXFLAGS) ../src/main.cpp shader.o -o ../bin/Matrix $(LIBS)

shader.o: ../src/shader.cpp ../src/shader.h
	$(CC) $(CXXFLAGS) -c ../src/shader.cpp ../src/shader.h

clean:
	$(RM) ../bin/Matrix
	$(RM) *.~ *.o