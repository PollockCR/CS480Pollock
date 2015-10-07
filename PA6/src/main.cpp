// removed idle function, not in use
// created seperate files for fragment and vertex shader
#include "shader.h" // header file of shader loaders
#include "mesh.h" // header file of object loader
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

// GLOBAL CONSTANTS
const char* vsFileName = "../bin/shader.vs";
const char* fsFileName = "../bin/shader.fs";
const char* blankTexture = "../../Resources/white.png";

// GLOBAL VARIABLES

  // Window size
  int w = 640, h = 480;

  // geomerty size
  int geometrySize;

  // The GLSL program handle
  GLuint program;
  GLuint vbo_geometry;
  GLuint texture;

  // rotations
  int orbit = -1;
  int rotation = -1;

  // uniform locations
  GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

  // attribute locations
  GLint loc_position;
  GLint loc_texture;

  // transform matrices
  glm::mat4 model;// obj-> world (planet) 
  glm::mat4 view;// world->eye
  glm::mat4 projection;// eye->clip
  glm::mat4 mvp;// premultiplied modelviewprojection

  // time information
  std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;

// FUNCTION PROTOTYPES

  //--GLUT Callbacks
  void render();

  // update display functions
  void update();
  void reshape(int n_w, int n_h);

  // called upon input
  void keyboard(unsigned char key, int x_pos, int y_pos);
  void manageMenus(bool quitCall);
  void menu(int id);
  void mouse(int button, int state, int x_pos, int y_pos);

  //--Resource management
  bool initialize( char* objectFilename, const char* textureFilename );
  void cleanUp();

  //--Time function
  float getDT();


// MAIN FUNCTION
int main(int argc, char **argv)
{
    bool init = false;
    // If the user didn't provide a filename command line argument,
    // print an error and exit.
    if (argc <= 1)
    {
        std::cout << "ERROR: Usage: " << argv[0] << " <Filename>. Please try again." << std::endl;
        exit(1);
    }

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);

    // Get filename of object
    char* objPtr  = argv[1];
    
    // Name and create the Window
    glutCreateWindow("Model Loader");

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

    // add menus
    manageMenus( false );

    // Initialize all of our resources(shaders, geometry)
    // pass blank texture if not given one 
    if( argc == 2 )
    {
      init = initialize( objPtr, blankTexture );
    }
    // or, pass texture given from command line arguments
    else
    {
      init = initialize( objPtr, argv[2] );
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
  // clear the screen
  glClearColor(0.0, 0.0, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // premultiply the matrix for this example
  mvp = projection * view * model;

  // enable the shader program
  glUseProgram(program);

  // upload the matrix to the shader
  glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, &mvp[0][0]); 

  // set up the Vertex Buffer Object so it can be drawn
  glEnableVertexAttribArray(loc_position);
  glEnableVertexAttribArray(loc_texture);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);

  // set pointers into the vbo for each of the attributes(position and color)
  glVertexAttribPointer( loc_position,//location of attribute
                         3,//number of elements
                         GL_FLOAT,//type
                         GL_FALSE,//normalized?
                         sizeof(Vertex),//stride
                         0);//offset

  glVertexAttribPointer( loc_texture,
                         2,
                         GL_FLOAT,
                         GL_FALSE,
                         sizeof(Vertex),
                         (void*)offsetof(Vertex,uv));

  glDrawArrays(GL_TRIANGLES, 0, geometrySize);//mode, starting index, count

  //clean up
  glDisableVertexAttribArray(loc_position);
  glDisableVertexAttribArray(loc_texture);
               
  //swap the buffers
  glutSwapBuffers();

}

// called on idle to update display
void update()
{
  // update object

    //total time
    static float rotationAngle = 0.0;
    static float orbitAngle = 0.0;

    float dt = getDT(); 

    // move object 90 degrees a second
    orbitAngle += dt * M_PI/2; // orbit
    rotationAngle += dt * M_PI/2; // rotate

    // rotation of cube around itself
    model = glm::rotate( glm::mat4(1.0f), rotationAngle, glm::vec3(0.0, 1.0, 0.0));
    model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));

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
  // Handle keyboard input - end program
  if(key == 27) // esc
  {
    glutLeaveMainLoop();
  }   
}

// initialize basic geometry and shaders for this example
bool initialize( char* objectFilename, const char* textureFilename )
{
    // define model with model loader
    bool geometryLoadedCorrectly;
    Mesh object;
    ShaderLoader programLoad;
    // Image blob
    Magick::Blob m_blob;


    // initialize magick
    Magick::InitializeMagick(NULL);

    // create an image pointer
    Magick::Image* m_pImage;

    // try to load image
    try
    {
      // save image to image pointer
      m_pImage = new Magick::Image( textureFilename );
    }
    // output error if not loaded
    catch(Magick::Error& err)
    {
      std::cerr << "[F] IMAGE NOT LOADED CORRECTLY: " << err.what() << std::endl;
      return false;
    }

    // write data to blob
    m_pImage->write(&m_blob, "RGBA");

    // load model into mesh object
    geometryLoadedCorrectly = object.loadMesh( objectFilename );

      // return false if not loaded
      if( !geometryLoadedCorrectly )
      {
        std::cerr << "[F] GEOMETRY NOT LOADED CORRECTLY" << std::endl;
        return false;
      }

    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, object.geometry.size()*sizeof(Vertex), &object.geometry[0], GL_STATIC_DRAW);

    // Create Texture object
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pImage->columns(), m_pImage->rows(), 0, GL_RGBA,     
                                                                GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    // save size of geometry
    geometrySize = object.geometry.size();

    //and its done
    return true;
}

// delete old items
void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);   
    glDeleteBuffers(1, &vbo_geometry);
    glDeleteBuffers(1, &texture);
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
    case 1:
      glutLeaveMainLoop();
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

