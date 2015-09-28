A simple example of matrix use in OpenGL
========================================

Explanation of the assignment from <http://www.cse.unr.edu/~fredh/class/480/F2015/proj/PA04/PA4.php>:

 Objective:

    Part 1: Create a model in Blender and export the OBJ file.
    Part 2: Model Loading: Write a model loader that can load your OBJ from Part 1 into your program. 

Part 1: Blender

    Create a board with four walls in Blender. Experiment with scaling, extruding, rotating, and other basic Blender features. The board should be exported as a single object. YouTube has many useful tutorials for Blender. When exporting the board, make sure to triangulate faces because your program handles triangles instead of quads. The board should be exported as a single object i.e. it should not be a bunch of rectangles next to each other.
    Extra Credit (required for Grad Students): Add colors (materials) to the board. 

Part 2: Model Loading

    Write a model loader that can load your OBJ from Part 1 into your program.
    What's needed:
        A Wavefront (.obj) file
        An existing program that places the model on the screen (Week 2 or 3 should help) 
    Your assignment is to write a model loader. It can be a function that takes the OBJ filename as a parameter and either returns the model or passes the model back as a reference parameter. Take a program that is using a struct to represent a vertex:

    	struct Vertex
    	{
    	   GLfloat position[3];
    	   GLfloat color[3];
    	};
    		

    where the three array indices of "position" stand for the x, y, and z components of the location, and the three indices of "color" are the red, green, and blue values of the vertex color, respectively. The colors can be hard coded by undergraduates. An array of these structs would be declared statically as:

    		Vertex geometry[NUM_VERTICES];
    		

    This array "geometry" holds the vertices that are passed into the vertex shader. Following a format similar to this, a function definition for the model loader could be:

    		bool loadOBJ(char * obj, Vertex geometry[]);
    		

    where "obj" is the filename of the OBJ file and the function returns false if there was a problem with the file.
    Instead of hardcoding the vertices of an object as done in the previous projects, these vertices can now be taken from the OBJ file. Any line that begins with a "v" refers to a vertex, and the three values following the "v" are the x, y, and z locations of this vertex. The model loader needs to take these values and assign them to the "Vertex" objects in the "geometry" array.
    Examples of how this is done can be found at THIS TUTORIAL. This tutorial also explains how to read an OBJ file and some options for exporting the OBJ from Blender. Note that you may need to export your model with triangulated faces if your program is drawing triangles. The goal of this assignment is to be able to create a model in Blender and load it into your program.  

Libraries being used:
-All libraries being used are the same from PA1: 
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include <fstream>
#include "shaderClass.h"
#include "shaderClass.cpp"

How to run the program:

Building This Example
---------------------

*This example requires GLM*
*On ubuntu it can be installed with this command*

>$ sudo apt-get install libglm-dev

*On a Mac you can install GLM with this command(using homebrew)*
>$ brew install glm

To build this example just 

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin

The file path for the obj model needs to be added as a runtime argument

Example: (if you are in the bin directory)

>$ ./Matrix sandbox.obj

Example: (if you are in the build directory)

>$ ../bin/Matrix ../bin/sandbox.obj

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
