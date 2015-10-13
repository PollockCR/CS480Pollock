// removed idle function, not in use
// created seperate files for fragment and vertex shader
#include "shader.h" // header file of shader loaders
#include "mesh.h" // header file of object loader
#include "planet.h" // header file for planet class
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <cstring>
#include <vector>

// Assimp
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/color4.h> // Post processing flags

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // makes passing matrices to shaders easier

// MAGICK++
#include <Magick++.h>

// DATA TYPES
/*struct ViewData
{
  int currentView = 0;
  bool newView = false;
  bool updateViewFlag = false;
  float currentPos[3];
  float destPos[3];
  float currentFocus[3];
  float destFocus[3];  
};*/

// GLOBAL CONSTANTS
const char* vsFileName = "../bin/shader.vs";
const char* fsFileName = "../bin/shader.fs";
const char* defaultInfo = "../bin/planetinfo.txt";
const char* defaultInfoActual = "../bin/planetinfo_actual.txt";
const char* blankTexture = "../../Resources/white.png";

// GLOBAL VARIABLES

  // Window size
  int w = 640, h = 480;

  // The GLSL program handle
  GLuint program[2];
  GLuint textProgram;
  bool updateViewFlag = false;
  int currentView = 0;

  // flags
  int mode = 0;
  bool displayMenu = true;

  // uniform locations
  GLint loc_mvpmat[2];// Location of the modelviewprojection matrix in the shader

  // attribute locations
  GLint loc_position[2];
  GLint loc_texture[2];

  // transform matrices
  glm::mat4 view;// world->eye
  glm::mat4 projection;// eye->clip
  glm::mat4 temp;

  // planets
  std::vector<Planet> planets;
  int numPlanets = 0;

  // time information
  std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;

// FUNCTION PROTOTYPES

  //--GLUT Callbacks
  void render();
  void displayText();
  void sPrint( float xPos, float yPos, const char *str);

  // update display functions
  void update();
  void changeView();
  void reshape(int n_w, int n_h);

  // called upon input
  void keyboard(unsigned char key, int x_pos, int y_pos);
  void special(int key, int xPos, int yPos);  
  void manageMenus(bool quitCall);
  void menu(int id);
  void mouse(int button, int state, int x_pos, int y_pos);

  //--Resource management
  bool initialize( const char* scaledFilename, const char* actualFilename );
  bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes, int offset );
  void cleanUp();

  //--Time function
  float getDT();


// MAIN FUNCTION
int main(int argc, char **argv)
{
    bool init = false;

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    // Name and create the Window
    glutCreateWindow("Solar System");

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutSpecialFunc(special); // Called if there is special input (arrows)


    // add menus
    manageMenus( false );

    // Initialize all of our resources(shaders, geometry)
    // pass default planet info if not given one 
    if( argc != 3 )
    {
      init = initialize( defaultInfo, defaultInfoActual );
    }
    // or, pass planet info given from command line argument
    else
    {
      init = initialize( argv[1], argv[2] );
    }

    // if initialized, begin glut main loop
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // remove menus
    manageMenus( true );

    // clean up and end program
    cleanUp();
    return 0;
}

// FUNCTION IMPLEMENTATION

// render the scene
void render()
{
  int index;
  //int pIndex;
  int offset = 0; 
  // clear the screen
  glClearColor(0.0, 0.0, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // display menu if enabled
  if(displayMenu)
  {
    displayText();
  }


  // set offset for planets (scaled = 0, actual = numPlanets)
  if( mode == 1 )
  {
    offset = numPlanets;
  }

  // enable the shader program
  glUseProgram(program[mode]);  

  // loop through each planet
  for( index = 0; index < numPlanets; index++ )
  {
    // premultiply the matrix for this example
    planets[index+offset].mvp = projection * view * planets[index+offset].model;

    // upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat[mode], 1, GL_FALSE, &(planets[index+offset].mvp[0][0])); 

    // set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position[mode]);
    glBindBuffer(GL_ARRAY_BUFFER, planets[index+offset].vbo_geometry);

    // set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position[mode],//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glEnableVertexAttribArray(loc_texture[mode]);
    glBindTexture(GL_TEXTURE_2D, planets[index+offset].texture);

    glVertexAttribPointer( loc_texture[mode],
                           2,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,uv));

    glDrawArrays(GL_TRIANGLES, 0, planets[index+offset].geometrySize);//mode, starting index, count
  }

  //clean up
  glDisableVertexAttribArray(loc_position[mode]);
  glDisableVertexAttribArray(loc_texture[mode]);
               
  //swap the buffers
  glutSwapBuffers();    
}

// displays text on screen
void displayText()
{
  // place text 
  glUseProgram(textProgram);
  char* textMode = new char[100];
  std::string textPlanet = "Current View - L/R Arrows: " + planets[currentView].nameOfPlanet;
  if( mode == 0 )
  {
    textMode = (char*)"Mode - Spacebar: Scaled view";
  }
  else 
  {
    textMode = (char*)"Mode - Spacebar: Actual view";
  }
  sPrint(-0.95, 0.90, (char*)"Quit - Esc");
  sPrint(-0.95, 0.80, (char*)"Toggle Menu Display - m");
  sPrint(-0.95, 0.70, textMode);
  sPrint(-0.95, 0.60, textPlanet.c_str());  

}

// prints a string to the screen
void sPrint( float xPos, float yPos, const char *str)
{
  int length;
  int index;

  // see how many characters are in text string.
  length = strlen( str ); 

  // location to start printing text
  glRasterPos2f(xPos, yPos); 

  // loop until index is greater then length
  for( index = 0; index < length; index++) 
  {
    // Print a character on the screen
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[index]); 
  }
}

// called on idle to update display
void update()
{
  // update object
  int index;
  int offset = 0;

  //total time
  float dt = getDT(); 

  // set offset for planets (scaled = 0, actual = numPlanets)
  if( mode == 1 )
  {
    offset = numPlanets;
  }  

  // loop through each planet
  for( index = 0; index < numPlanets; index++ )
  {
    // move object orbitSpeed radians a second
    planets[index+offset].orbitAngle += dt * planets[index+offset].orbitSpeed;
    // move object rotationSpeed radians a second
    planets[index+offset].rotationAngle += dt * planets[index+offset].rotationSpeed;

    // check for moon 
    if( planets[index+offset].orbitIndex == 0 )
    {
      // orbit of planet
      planets[index+offset].model = glm::translate(glm::mat4(1.0f),
          glm::vec3(planets[index+offset].orbitPath.x * sin(planets[index+offset].orbitAngle),
                    planets[index+offset].orbitPath.y,
                    planets[index+offset].orbitPath.z * cos(planets[index+offset].orbitAngle)));
    }
    else
    {
      // orbit of planet
      planets[index+offset].model = glm::translate(glm::mat4(1.0f),
          glm::vec3(planets[index+offset].orbitPath.x * sin(planets[index+offset].orbitAngle),
                    planets[index+offset].orbitPath.y,
                    planets[index+offset].orbitPath.z * cos(planets[index+offset].orbitAngle))) * planets[planets[index+offset].orbitIndex].model;
    }


    // rotation of planet
    planets[index+offset].model = glm::rotate( planets[index+offset].model, planets[index+offset].rotationAngle, planets[index+offset].rotationAxis);
  }

  // change the view
  changeView();


  // update the state of the scene
  glutPostRedisplay();//call the display callback   
}

// change which planet to view
void changeView()
{
  //int index;
  int offset = 0;
  if( mode == 1 )
  {
    offset = numPlanets - 1;
  }

  //static float currentPos[3];
  //static float currentFocus[3];
  static float destPos[3];
  static float destFocus[3];

  switch( currentView )
  {
    // default view
    case 0:
      updateViewFlag = false;
      view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                          glm::vec3(0.0, 0.0, 0.0), //Focus point
                          glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
      break;
    default:
      destPos[0] = (4 + planets[currentView+offset].orbitPath.x) * sin(planets[currentView+offset].orbitAngle);
      destPos[1] = planets[currentView+offset].orbitPath.y;        
      destPos[2] = (4 + planets[currentView+offset].orbitPath.z) * cos(planets[currentView+offset].orbitAngle);        
      destFocus[0] = planets[currentView+offset].orbitPath.x * sin(planets[currentView+offset].orbitAngle);        
      destFocus[1] = planets[currentView+offset].orbitPath.y;        
      destFocus[2] = planets[currentView+offset].orbitPath.z * cos(planets[currentView+offset].orbitAngle);            
    /*
      currentPos[0] = (4 + planets[currentView+offset-1].orbitPath.x) * sin(planets[currentView+offset].orbitAngle);
      currentPos[1] = planets[currentView+offset-1].orbitPath.y;
      currentPos[2] = (4 + planets[currentView+offset-1].orbitPath.z) * cos(planets[currentView+offset].orbitAngle);
      currentFocus[0] = planets[currentView+offset-1].orbitPath.x * sin(planets[currentView+offset].orbitAngle);
      currentFocus[1] = planets[currentView+offset-1].orbitPath.y; 
      currentFocus[2] = planets[currentView+offset-1].orbitPath.z * cos(planets[currentView+offset].orbitAngle);

      if( updateViewFlag )
      {
        for( index = 0; index < 3; index++ )
        {
          if( currentPos[index] < destPos[index] )
          {
            currentPos[index] += 0.15;
          }
          if( currentFocus[index] > destFocus[index] )
          {
            currentPos[index] -= 0.15;
          }

          if( currentFocus[index] < destFocus[index] )
          {
            currentFocus[index] += 0.15;
          }
          if( currentFocus[index] > destFocus[index] )
          {
            currentFocus[index] -= 0.15;
          }          
        }

        if( (currentPos[0] - destPos[0]) <= 0.5 && (currentPos[0] - destPos[0]) >= -0.5 &&
            (currentPos[1] - destPos[1]) <= 0.5 && (currentPos[1] - destPos[1]) >= -0.5 &&            
            (currentPos[2] - destPos[2]) <= 0.5 && (currentPos[2] - destPos[2]) >= -0.5 &&
            (currentFocus[0] - destFocus[0]) <= 0.5 && (currentFocus[0] - destFocus[0]) >= -0.5 &&
            (currentFocus[1] - destFocus[1]) <= 0.5 && (currentFocus[1] - destFocus[1]) >= -0.5 &&            
            (currentFocus[2] - destFocus[2]) <= 0.5 && (currentFocus[2] - destFocus[2]) >= -0.5 )
        {
          updateViewFlag = false;
          view = temp;
        }

        view = glm::lookAt( glm::vec3(currentPos[0], currentPos[1], currentPos[2]), 
                      glm::vec3(currentFocus[0], currentFocus[1], currentFocus[2]), 
                      glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up          
      }
      else
      {
    */
      view = glm::lookAt(glm::vec3(destPos[0], destPos[1], destPos[2]), 
                           glm::vec3(destFocus[0], destFocus[1], destFocus[2]), 
                           glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
     // }
      break;
  }

  // update the state of the scene
  glutPostRedisplay();//call the display callback   
}

// resize window
void reshape(int n_w, int n_h)
{
    // set new window width and height
    w = n_w;
    h = n_h;

    // change the viewport to be correct
    glViewport( 0, 0, w, h);

    // update the projection matrix
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

// called on keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos )
{
  // Handle keyboard input
  switch(key)
  {
    //end program
    case 27: // esc
      glutLeaveMainLoop();
      break; 
  // switch modes
    case 32: // space bar
      if( mode == 0 )
      {
        mode = 1;
      }
      else 
      {
        mode = 0;
      }
      break;
    case 77:
    case 109:
      displayMenu = !displayMenu;
      break;
    default:
      break;
  }   

}

void special(int key, int xPos, int yPos)
{
  // reset view 
  if( key == GLUT_KEY_LEFT)
  {
    currentView = 0;
    updateViewFlag = true;        
  }
  // go to next view
  else if( key == GLUT_KEY_RIGHT)
  {
    // increment planet view index
    currentView++;

    // skip moons
    if( planets[currentView].orbitIndex != 0 )
    {
      currentView++;
    }

    // reset if at end
    if( currentView >= numPlanets - 1 )
    {
      currentView = 0;
    }      

    // flag to update view
    updateViewFlag = true;
  }  
}

// initialize basic geometry and shaders for this example
bool initialize( const char* scaledFilename, const char* actualFilename )
{
    // define model with model loader
    int pIndex;
    int index;
    int offset = 0;
    std::vector<Mesh> meshes;
    ShaderLoader programLoad;

    // load scaled planet info
    if( !loadInfo( scaledFilename, meshes, 0 ) )
    {
      return false;
    }

    // load actual planet info
    if( !loadInfo( actualFilename, meshes, numPlanets ) )
    {
      return false;
    }      

    for( pIndex = 0; pIndex < 2; pIndex++ )
    {
      if( pIndex == 1 )
      {
        offset = numPlanets;
      }

      // loop through each planet
      for( index = 0; index < numPlanets; index++ )
      {
        // Create a Vertex Buffer object to store this vertex info on the GPU
        glGenBuffers(1, &(planets[index+offset].vbo_geometry));
        glBindBuffer(GL_ARRAY_BUFFER, planets[index+offset].vbo_geometry);
        glBufferData(GL_ARRAY_BUFFER, meshes[index+offset].geometry.size()*sizeof(Vertex), &(meshes[index+offset].geometry[0]), GL_STATIC_DRAW);

        // Create Texture object
        glGenTextures(1, &(planets[index+offset].texture));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planets[index+offset].texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, planets[index+offset].imageCols, planets[index+offset].imageRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, planets[index+offset].m_blob.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);        
      }      

      // loads shaders to program
      programLoad.loadShader( vsFileName, fsFileName, program[pIndex] );


      // Get a handle for our "MVP" uniform
      loc_mvpmat[pIndex] = glGetUniformLocation(program[pIndex], "mvpMatrix");
        if(loc_mvpmat[pIndex] == -1)
        {
          std::cerr << "[F] MVP MATRIX NOT FOUND" << std::endl;
          return false;
        }       

      // Get a handle for our buffers
      loc_position[pIndex] = glGetAttribLocation(program[pIndex], "v_position");
        if(loc_position[pIndex] == -1)
        {
          std::cerr << "[F] POSITION NOT FOUND" << std::endl;
          return false;
        }

      // get a handle for our texture
      loc_texture[pIndex] = glGetAttribLocation(program[pIndex], "v_color");
        if(loc_texture[pIndex] == -1)
        {
          std::cerr << "[F] COLOR NOT FOUND" << std::endl;
          return false;
        }            
    }


    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

// load info from file into planets
bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes, int offset )
{
  // initialize variables
  std::ifstream ifs(infoFilepath, std::ifstream::in);
  bool geometryLoadedCorrectly;
  bool imageLoadedCorrectly;
  int index;
  int numInfo = -1;
  GLfloat scale;

  // check for open file
  if( !ifs.is_open() )
  {
    std::cerr << "[F] FAILED TO READ FILE!" << infoFilepath << std::endl;
    return false;    
  }

  // read in number of planets
  ifs >> numInfo;

  // otherwise, compare number of planets and set offset
  if( offset != 0 )
  {
    if( numInfo != offset )
    {
      std::cerr << "[F] FAILED TO READ FILE! DIFFERENT NUMBER OF PLANETS" << std::endl;
      return false;  
    }
  }

  // check for invalid file format
  if( numInfo == -1 )
  {
    std::cerr << "[F] FAILED TO READ FILE! INVALID FORMAT" << std::endl;
    return false; 
  }

  // loop through each planet
  for( index = 0; index < numInfo; index++ )
  {
    // create new planet
    Planet tempPlanet;
    planets.push_back(tempPlanet);

    // create new mesh
    Mesh tempMesh;
    meshes.push_back(tempMesh);

    // read from file

      // load obj file
      std::string objFilepath;
      ifs >> objFilepath;
      ifs >> scale;
      geometryLoadedCorrectly = meshes[index+offset].loadMesh( objFilepath.c_str(), scale );

        // return false if not loaded
        if( !geometryLoadedCorrectly )
        {
          std::cerr << "[F] GEOMETRY NOT LOADED CORRECTLY" << std::endl;
          return false;
        }

      // load texture
      std::string textureFilepath;
      ifs >> textureFilepath;
      imageLoadedCorrectly = planets[index+offset].loadImage(textureFilepath.c_str());

        // return false if not loaded
        if( !imageLoadedCorrectly )
        {
          return false;
        }

      // load texture name
      ifs >> planets[index+offset].nameOfPlanet;

      // load rotation speed
      ifs >> planets[index+offset].rotationSpeed;

      // load orbit speed
      ifs >> planets[index+offset].orbitSpeed;

      // load rotation axis
      ifs >> planets[index+offset].rotationAxis.x >> planets[index+offset].rotationAxis.y >> planets[index+offset].rotationAxis.z;

      // load orbit path 
      ifs >> planets[index+offset].orbitPath.x >> planets[index+offset].orbitPath.y >> planets[index+offset].orbitPath.z;

      // load index of planet to orbit
      ifs >> planets[index+offset].orbitIndex;

      // save size of geometry
      planets[index+offset].geometrySize = meshes[index+offset].geometry.size();
    
  }

  // update planet count
  if( offset == 0 )
  {
    numPlanets = planets.size(); 

    // make sure data matches
    if( numInfo != numPlanets )
    {
      std::cerr << "[F] FAILED TO READ FILE! DATA DOES NOT MATCH PLANET COUNT" << std::endl;
      return false; 
    }       
  }

  // return success
  return true;
}

// delete old items
void cleanUp()
{
  // initialize variables
  int index;

  // clean up programs
  glDeleteProgram(program[0]); 
  glDeleteProgram(program[1]);   

    // clean up each planet
    for( index = 0; index < numPlanets*2; index++ )
    {
      glDeleteBuffers(1, &(planets[index].vbo_geometry));
      glDeleteBuffers(1, &(planets[index].texture));      
    }
}

// adds and removes menus
void manageMenus( bool quitCall )
{
  int main_menu = 0;

  // upon initialization
  if( !quitCall )
  {
    // create main menu
    main_menu = glutCreateMenu(menu); // Call menu function
    glutAddMenuEntry("Quit", 1);
    glutAddMenuEntry("Switch Mode", 2);
    glutAddMenuEntry("Toggle Menu Display", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON); //Called if there is a mouse click (right)
  }

  // destroy menus before ending program
  else
  {
    // clean up after ourselves
    glutDestroyMenu(main_menu);
  }

  // update display
  glutPostRedisplay();
}

// menu choices 
void menu(int id)
{
  // switch case for menu options
  switch(id)
  {
    // call the rotation menu function
    case 1: // exit
      glutLeaveMainLoop();
      break;
    case 2: // swap mode
      if( mode == 0 )
      {
        mode = 1;
      }
      else 
      {
        mode = 0;
      }
      break;
    case 3: // toggle menu display
      displayMenu = !displayMenu;
      break;
    // default do nothing
    default:
      break;
  }
  // redraw screen without menu
  glutPostRedisplay();
}

// actions for left mouse click
void mouse(int button, int state, int x_pos, int y_pos)
{
  // redraw screen without menu
  glutPostRedisplay();
}

//returns the time delta
float getDT()
{
    float ret;

    // update time using time elapsed since last call
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}

