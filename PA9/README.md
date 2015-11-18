Air Hockey
==========

Project Authors
---------------
This is the repository for the group Catherine Pollock, Conor Sullivan, and Peter Rahmanifar

Program Function
----------------
The program is an interactive air hockey game. An information file can be passed through the command line, or default ones will be used. Object information file must be in the following format:

```
int numberOfObjects
string filepathToObject1
string filepathToTexture1
string filepathToObject2
string filepathToTexture2
...
```

Object information files must be in correct format or undesired results will occur. All files mentioned must be in location specified in info file. 

When the program starts, press spacebar to start or Esc to quit.

When the program is running, the following commands can be used:

- Mouse/WASD to Move Player 1 Paddle
- Arrow Keys to Move Player 2 Paddle (When AI is disabled)
- K to Pan to Player 1 POV (Default)
- I to Pan to Player 2 POV
- J to Pan to Left Side of Board
- L to Pan to Right Side of Board
- Spacebar to Pause/Resume
- G to Switch Player 1 Controls (Mouse/WASD)
- Right Click for:
  - Change View 
  - Pause/Resume Game
  - Switch Player 1 Controls
  - Enable/Disable AI
  - Restart Game
  - Quit     
- H to Hide Menu
- Esc to Quit

Note: The keyboard controls do not change with the view. The mouse controls changes with view. AI only works from Player 1 POV.

Project discripton found here: http://www.cse.unr.edu/~fredh/class/480/F2015/proj/PA09/PA9.php

Extra Credit
------------
The following features were added beyond project requirements:

- 2D Text Display
- Toggle Menu Display On/Off
- 2 Humans or Human/AI
- Toggle for AI/Player 2 Control
- Multiple AI Levels
- Game Replay

Ubuntu Dependencies
-------------------
The following packages can be installed with these commands:

*This project requires bullet* 

>$ sudo apt-get install libbullet-dev

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

*This project requires bullet*

>$ brew install bullet

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

To clean repository:

>$ cd build

>$ make clean

To build project:

>$ cd build

>$ make

If you are using a Mac you will need to edit the makefile in the build directory

The excutable will be put in bin. The program must be ran from inside build or bin.

To run the program, execute these commands, replacing with your program's object information filepath:

>$ ../bin/Model

*or*

>$ ../bin/Model ../bin/imageInfo.txt 
