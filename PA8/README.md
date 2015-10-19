OpenGL Model Loader with Textures and Assimp
============================================

Project Authors
---------------
This is the repository for the group Catherine Pollock, Conor Sullivan, and Peter Rahmanifar

Program Function
----------------
The program will load a model provided to it by the command line, along with its texture file (if applicable). The model will be loaded using assimp and then displayed with the supplied texture. If no texture is supplied, the model will be white.

Project discripton found here: http://www.cse.unr.edu/~fredh/class/480/F2015/proj/PA06/PA6.php

Extra Credit
------------
Extra credit was not offered in this project.

Ubuntu Dependencies
-------------------

The following packages can be installed with these commands:

*This project requires magick++* 

>$ sudo apt-get install libmagick++-dev imagemagick

*This project requires assimp3* 

>$ sudo apt-get install libassimp-dev

*This project requires GLUT and GLEW* 

>$ sudo apt-get install freeglut3-dev freeglut3 libglew1.6-dev

*This project requires GLM*

>$ sudo apt-get install libglm-dev

Mac OSX Dependencies
--------------------

The following libraries can be installed with these commands using homebrew:

*This project requires magick++*

>$ brew install imagemagick

*This project requires assimp3* 

>$ brew install assimp

*This project requires GLUT and GLEW* 

>$ brew install freeglut glew

*This project requires GLM*

>$ brew install glm

Additionally, ensure that the latest version of the Developer Tools is installed

Building this Project
---------------------

To build this project just 

>$ cd build

>$ make

If you are using a Mac you will need to edit the makefile in the build directory

The excutable will be put in bin

To run the program, execute these commands, replacing with your object filepath and your object's texture filepath:

>$ cd ../bin/

>$ ./Model  ~/Desktop/capsule.obj ~/Desktop/capsule0.jpg

*or*

>$ cd ../bin/

>$ ./Model  ~/Desktop/capsule.obj
