A simple example of matrix use in OpenGL
========================================

Explanation of the assignment from <http://www.cse.unr.edu/~fredh/class/480/F2015/proj/PA03/PA3.php>:

 Objective: Now that we have a window open and an animation going, with keyboard, mouse, and menus, we will be dealing with multiple objects. You'll be taking your code from last week and a moon to the object you have already.

For this project, students will be expected to render two cubes onto the screen. One of the cubes will move around in a circle while the other cube orbits around it. The program is expected to be designed in such a way that the user, through keyboard inputs, can change the direction of rotation of the first cube, while the second continuously orbits it.

    In addition to the planet orbiting and the moon orbiting the planet properly (no matter which direction it is going) you need to implement the planet direction changes with the keyboard arrows (in addition to the keyboard/mouse done last time).
    A Recommended tutorial on matricies (from previous students) can be found here
    Extra Credit (Required for Grad Students): Add text to indicate which direction the planet is rotating. This should change when the direction changes. 

Libraries being used:
-All libraries being used are the same from PA1: 
#include <GL/glew.h>
#include <GL/glut.h> // doing otherwise causes compiler shouting
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
-Once make is run in the build folder, a Matrix program will appear in the bin folder. In order to execute, double click on the program or right click and choose the "Run" option. A screen will appear with a stationary cube. If you right click, a menu will appear that will have a "Quit" option, which will end the program, and a "start/stop rotation" option. When the latter is highlighted, a submenu will appear with an option to either "start rotation"
or "stop rotation". The former of the two will initiate the rotation of the planet in its orbit. The latter will freeze the object on the screen. The left mouse click reveals a menu with three options: "Quit", "reverse rotation", and "reverse orbit". "Quit" simply ends the program. The "reverse rotation" option reverses the rotation of the object about its own y-axis. The "reverse orbit" option reverses the direction of the orbit the object is currently going in. These opertions can also be done with the keyboard. The rotation of the object about its y-axis can be reversed by pressing 'A' or 'a' on the keyboard. Pressing 'B' or 'b' on the keyboard will reverse the orbit of the object. All reversals can be applied whether the object is moving or is still. Pressing 'esc' on the keyboard will end the program.

-Update on how to run program from PA2:
The first addition to this program is the smaller moon that is rotating around the larger planet. The rotation and orbit of the moon and planet can be reversed with the left mouse click. Also, the 'Q' and 'q' keyboard input reverse the moon's rotation and 'W' and 'w' keyboard input reverse the moon's orbit. Lastly, the left arrow on the keyboard reverses the planet's orbit and the right arrow reverses the planet's rotation.

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

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
