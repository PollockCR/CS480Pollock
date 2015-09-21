An 
=====================================================

Program Function
----------------
The program will display a rainbow, rotating cube. Another, smaller cube rotates the first cube in an orbit. A menu is available to start spinning, stop spinning, and to exit the program. The rotation of cubes and orbits can be changed from this menu. To open the menu, right click the mouse.Any left mouse click, left or right arrow keys, or any standard key press (except Esc or Ctrl-C) will reverse the rotation or the cube.

Extra Credit
------------
Extra credit was NOT achieved in this project.

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

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin
*To run the program, execute this command*
>$ ../bin/Matrix

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
