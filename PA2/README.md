An Interactive Rotating Cube in OpenGL
======================================

Program Function
----------------
The program will display a rainbow, rotating cube. A menu is available to start spinning,
stop spinning, and to exit the program. Any mouse click or key press (except Esc or Ctrl-C)
will reverse the rotation or the cube.

Building This Program
---------------------

*This example requires GLM*
*On Ubuntu it can be installed with this command*

>$ sudo apt-get install libglm-dev

*On a Mac you can install GLM with this command(using homebrew)*
>$ brew install glm

To build this example just 

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin
*To run the program, execute this command*
>$ ../bin/Matrix

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
