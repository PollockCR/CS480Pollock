Solar System Model
==================

Project Authors
---------------
This is the repository for the group Catherine Pollock, Conor Sullivan, and Peter Rahmanifar

Program Function
----------------
The program models the solar system. Planet information files can be passed through the command line, or default ones will be used. Two files are loaded from, one with scaled planet data and one with actual planet data. Scaled mode provides exaggerated planet sizes and orbits for better user viewing. Actual mode provides a view that demonstrates correct ratios in relation to the solar system. Planet information files must be in the following format:

```
int numberOfPlanets
string filepathToObject
GLfloat scaleOfPlanet (1.0 for sun, 0.5 for half of sun, etc)
string filepathToTexture
string nameOfTexture (no spaces)
float rotationSpeed
float orbitSpeed
glm::vec3 rotationAxis (i.e. "0.0 1.0 0.0" for rotation on y axis)
glm::vec3 orbitPath (i.e. "4.0 0.0 4.0" for 4 units x/z axis)
int orbitIndex (for index of object orbiting around, 0 for sun)
```

Planet information files must be in correct format or undesired results will occur. All files mentioned must be in location specified in planet info files. 

When the program is running, the following commands can be used:

- To quit: *Esc*
- To toggle menu display: *m*
- To change view mode: *v*
- To pause simulation: *spacebar*
- To view each planet: *left-arrow* to reset or *right-arrow* to go to next planet

Project discripton found here: http://www.cse.unr.edu/~fredh/class/480/F2015/proj/PA07/PA7.php

Extra Credit
------------
The following features were added beyond project requirements:
- Option to go from actual data to scaled view (v)
- Displays name of current planet view and current mode (right arrow)
- Camera pans from planet to planet
- Menu display can be toggled on/off (m)

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

The excutable will be put in bin. The program must be ran from inside build or bin.

To run the program, execute these commands, replacing with your object filepath and your object's texture filepath:

>$ cd ../bin/
>$ ./Model

*or*

>$ cd ../bin/
>$ ./Model ~/planetinfo.txt ~/planetinfo_actual.txt
