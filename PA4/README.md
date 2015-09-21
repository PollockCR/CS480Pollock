OpenGL Model Loader
===================

Program Function
----------------
The program will load a model provided to it by the command line. The model must be a .obj file and be properly configured. 

Extra Credit
------------
Extra credit was attempted in this project and materials were added to the board in the given object file.

Building This Program
---------------------

*This example requires GLUT and GLEW* 

*On ubuntu they can be installed with this command*

>$ sudo apt-get install freeglut3-dev freeglut3 libglew1.6-dev

*This example also requires GLM*
*On Ubuntu it can be installed with this command*

>$ sudo apt-get install libglm-dev

*On a Mac you can get these libraries with this command(using homebrew)*
>$ brew install freeglut glew

*On a Mac you can install GLM with this command(using homebrew)*
>$ brew install glm

To build this example just 

>$ cd build
>$ make

You will be prompted to enter the file path to the object file. Example response:

>$ ../bin/table.objs

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin
*To run the program, execute this command*
>$ ../bin/Model

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
