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
const char* defaultInfo = "../bin/objectinfo.txt";
const char* blankTexture = "../../Resources/white.png";

// GLOBAL VARIABLES

  // Window size
  int w = 640, h = 480;

  // The GLSL program handle
  GLuint program;
  GLuint textProgram;
  float dt = 0.0;

  // flags
  bool displayMenu = true;

  // uniform locations
  GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

  // attribute locations
  GLint loc_position;
  GLint loc_texture;

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
  void reshape(int n_w, int n_h);

  // called upon input
  void keyboard(unsigned char key, int x_pos, int y_pos);
  void special(int key, int xPos, int yPos);  
  void manageMenus(bool quitCall);
  void menu(int id);
  void mouse(int button, int state, int x_pos, int y_pos);

  //--Resource management
  bool initialize( const char* scaledFilename );
  bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes );
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
    glutCreateWindow("Bullet Demonstration");

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
    if( argc != 2 )
    {
      init = initialize( defaultInfo );
    }
    // or, pass planet info given from command line argument
    else
    {
      init = initialize( argv[1] );
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
  // clear the screen
  glClearColor(0.2, 0.2, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // display menu if enabled
    if(displayMenu)
    {
      displayText();
    }

    // enable the shader program
    glUseProgram(program);  


    // loop through each planet
    for( index = 0; index < numPlanets; index++ )
    {
      // premultiply the matrix for this example
      planets[index].mvp = projection * view * planets[index].model;

      // upload the matrix to the shader
      glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, &(planets[index].mvp[0][0])); 

      // set up the Vertex Buffer Object so it can be drawn
      glEnableVertexAttribArray(loc_position);
      glBindBuffer(GL_ARRAY_BUFFER, planets[index].vbo_geometry);

      // set pointers into the vbo for each of the attributes(position and color)
      glVertexAttribPointer( loc_position,//location of attribute
                             3,//number of elements
                             GL_FLOAT,//type
                             GL_FALSE,//normalized?
                             sizeof(Vertex),//stride
                             0);//offset

      glEnableVertexAttribArray(loc_texture);
      glBindTexture(GL_TEXTURE_2D, planets[index].texture);

      glVertexAttribPointer( loc_texture,
                             2,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof(Vertex),
                             (void*)offsetof(Vertex,uv));

      glDrawArrays(GL_TRIANGLES, 0, planets[index].geometrySize);//mode, starting index, count
    }

    

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texture);
                   
    //swap the buffers
    glutSwapBuffers();   
}

// displays text on screen
void displayText()
{
  // place text 
  glUseProgram(textProgram);
  sPrint(-0.95, 0.90, (char*)"Quit - Esc");
  sPrint(-0.95, 0.80, (char*)"Toggle Menu Display - m"); 
  sPrint(-0.95, 0.70, (char*)"More Options - Right Click");  

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

    //total time
    dt = getDT(); 

    // loop through each planet
    for( index = 0; index < numPlanets; index++ )
    {
      // move object orbitSpeed radians a second
      planets[index].orbitAngle += dt * planets[index].orbitSpeed;
      // move object rotationSpeed radians a second
      planets[index].rotationAngle += dt * planets[index].rotationSpeed;

        // orbit of planet
        planets[index].model = glm::translate(glm::mat4(1.0f),
            glm::vec3(planets[index].orbitPath.x * sin(planets[index].orbitAngle),
                      planets[index].orbitPath.y,
                      planets[index].orbitPath.z * cos(planets[index].orbitAngle)));

      // rotation of planet
      planets[index].model = glm::rotate( planets[index].model, planets[index].rotationAngle, planets[index].rotationAxis);
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
      break;
    case 77:
    case 109:
      displayMenu = !displayMenu;
      break;
    case 118:
    case 86:
      break;
    default:
      break;
  }   

}

void special(int key, int xPos, int yPos)
{

}

// initialize basic geometry and shaders for this example
bool initialize( const char* scaledFilename )
{
    // define model with model loader
    int index;
    std::vector<Mesh> meshes;
    ShaderLoader programLoad;

    // load scaled planet info
    if( !loadInfo( scaledFilename, meshes ) )
    {
      return false;
    }
      
      // loop through each planet
      for( index = 0; index < numPlanets; index++ )
      {
        // Create a Vertex Buffer object to store this vertex info on the GPU
        glGenBuffers(1, &(planets[index].vbo_geometry));
        glBindBuffer(GL_ARRAY_BUFFER, planets[index].vbo_geometry);
        glBufferData(GL_ARRAY_BUFFER, meshes[index].geometry.size()*sizeof(Vertex), &(meshes[index].geometry[0]), GL_STATIC_DRAW);

        // Create Texture object
        glGenTextures(1, &(planets[index].texture));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planets[index].texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, planets[index].imageCols, planets[index].imageRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, planets[index].m_blob.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);        
      }      

      // loads shaders to program
      programLoad.loadShader( vsFileName, fsFileName, program );


      // Get a handle for our "MVP" uniform
      loc_mvpmat = glGetUniformLocation(program, "mvpMatrix");
        if(loc_mvpmat == -1)
        {
          std::cerr << "[F] MVP MATRIX NOT FOUND" << std::endl;
          return false;
        }       

      // Get a handle for our buffers
      loc_position = glGetAttribLocation(program, "v_position");
        if(loc_position == -1)
        {
          std::cerr << "[F] POSITION NOT FOUND" << std::endl;
          return false;
        }

      // get a handle for our texture
      loc_texture = glGetAttribLocation(program, "v_color");
        if(loc_texture == -1)
        {
          std::cerr << "[F] COLOR NOT FOUND" << std::endl;
          return false;
        }            


    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 15.0, -30.0), //Eye Position
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
bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes )
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
      geometryLoadedCorrectly = meshes[index].loadMesh( objFilepath.c_str(), scale );

        // return false if not loaded
        if( !geometryLoadedCorrectly )
        {
          std::cerr << "[F] GEOMETRY NOT LOADED CORRECTLY" << std::endl;
          return false;
        }

      // load texture
      std::string textureFilepath;
      ifs >> textureFilepath;
      imageLoadedCorrectly = planets[index].loadImage(textureFilepath.c_str());

        // return false if not loaded
        if( !imageLoadedCorrectly )
        {
          return false;
        }

      // load texture name
      ifs >> planets[index].nameOfPlanet;

      // load rotation speed
      ifs >> planets[index].rotationSpeed;

      // load orbit speed
      ifs >> planets[index].orbitSpeed;

      // load rotation axis
      ifs >> planets[index].rotationAxis.x >> planets[index].rotationAxis.y >> planets[index].rotationAxis.z;

      // load orbit path 
      ifs >> planets[index].orbitPath.x >> planets[index].orbitPath.y >> planets[index].orbitPath.z;

      // load index of planet to orbit
      ifs >> planets[index].orbitIndex;

      // save size of geometry
      planets[index].geometrySize = meshes[index].geometry.size();
    
  }
    numPlanets = planets.size(); 

    // make sure data matches
    if( numInfo != numPlanets )
    {
      std::cerr << "[F] FAILED TO READ FILE! DATA DOES NOT MATCH PLANET COUNT" << std::endl;
      return false; 
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
  glDeleteProgram(program); 
  glDeleteProgram(textProgram);  

    // clean up each planet
    for( index = 0; index < numPlanets; index++ )
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
    glutAddMenuEntry("Pause/Resume Simulation", 4);
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
      break;
    case 3: // toggle menu display
      displayMenu = !displayMenu;
      break;
    case 4:
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
  // update display
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


