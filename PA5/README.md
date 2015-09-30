OpenGL Model Loader with Assimp
===============================

Project Authors
---------------
This is the repository for the group Catherine Pollock, Conor Sullivan, and Peter Rahmanifar

Program Function
----------------
The program will load a model provided to it by the command line. The model will be loaded using assimp and then displayed. The model will be green and red.

Project discripton found here: http://www.cse.unr.edu/~fredh/class/480/F2015/proj/PA05/PA5.php

Extra Credit
------------
It was unclear if extra credit was offered based on the description. If extra credit is adding colors to the model, that was achieved.

Building This Program
---------------------

*This example requires assimp3* 
*On ubuntu they can be installed with this command*

>$ sudo apt-get install libassimp-dev

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

*If you are using a Mac you will need to edit the makefile in the build directory*

*The excutable will be put in bin

*To run the program, execute these commands, replacing with your object filepath*

>$ cd ../bin/

>$ ./Model  ~/Desktop/table.obj

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
