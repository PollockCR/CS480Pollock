# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL -lassimp -I/usr/include/bullet -lBulletDynamics -lBulletSoftBody -lBulletCollision -lLinearMath `Magick++-config --cppflags --cxxflags --ldflags --libs`

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/Model

../bin/Model: ../src/main.cpp shader.o mesh.o loadImage.o
	$(CC) $(CXXFLAGS) ../src/main.cpp shader.o mesh.o loadImage.o -o ../bin/Model $(LIBS)

shader.o: ../src/shader.cpp ../src/shader.h
	$(CC) $(CXXFLAGS) -c ../src/shader.cpp ../src/shader.h $(LIBS)

mesh.o: ../src/mesh.cpp ../src/mesh.h
	$(CC) $(CXXFLAGS) -c ../src/mesh.cpp ../src/mesh.h $(LIBS)

loadImage.o: ../src/loadImage.cpp ../src/loadImage.h
	$(CC) $(CXXFLAGS) -c ../src/loadImage.cpp ../src/loadImage.h $(LIBS)

clean:
	$(RM) ../bin/Model
	$(RM) *.~ *.o
	$(RM) ../src/*.gch*
