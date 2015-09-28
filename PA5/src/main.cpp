#include "shader.h" // header file of shader loaders
#include "mesh.h" // header file of object loader
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
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

//--Data types
// This object defines the attributes of a vertex(position, color, etc...)
/*struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};*/

//--Global constants
const char* vsFileName = "../bin/shader.vs";
const char* fsFileName = "../bin/shader.fs";

// Global variables

int w = 640, h = 480;// Window size
Mesh* m_pMesh;
GLuint program;// The GLSL program handle

ShaderLoader programLoad; // Load shader class

    // Quit call
    bool quitCall = false;

// filename string
char * objPtr;

// uniform locations
GLint loc_mvp;// Location of the modelviewprojection matrix in the shader
GLint loc_model;
GLint loc_view;

// attribute locations
GLint loc_position;
GLint loc_color;
GLint loc_uv;
GLint loc_normal;

// transform matrices
glm::mat4 model;// obj-> world (planet) 
glm::mat4 view;// world->eye
glm::mat4 projection;// eye->clip
glm::mat4 mvp;// premultiplied modelviewprojection

//--GLUT Callbacks
void render();

  // update display functions
  void update();
  void reshape(int n_w, int n_h);

  // called upon input
  void keyboard(unsigned char key, int x_pos, int y_pos);
  void manageMenus();
  void menu(int id);
  void mouse(int button, int state, int x_pos, int y_pos);

//--Resource management
bool initialize();
void cleanUp();



//--Main
int main(int argc, char **argv)
{
    // If the user didn't provide a filename command line argument,
    // print an error and exit.
    if (argc <= 1)
    {
        std::cout << "ERROR: Usage: " << argv[0] << " <Filename>" << std::endl;
        exit(1);
    }

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);

    // Get filename of object
    objPtr = argv[1];

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
    manageMenus();

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
    if(init)
    {
        glutMainLoop();
    }

    // clean up and end program
    cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //GLuint light = glGetUniformLocation(program, "light_pos");

    // render first object

      //premultiply the matrix for this example
      mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(loc_model, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(loc_view, 1, GL_FALSE, &view[0][0]);   

      /*// light
      glm::vec3 lightPos = glm::vec3(0,4,4);
      glUniform3f(light, lightPos.x, lightPos.y, lightPos.z);*/
                 
    //swap the buffers
    glutSwapBuffers();

}

// called on idle to update display
void update()
{
    // check for quit program
    if( quitCall )
    {
      manageMenus();
      exit(0);
    }
    // otherwise, continue display
    else 
    {
      // update the state of the scene
      glutPostRedisplay();//call the display callback
    }
}

// resize window
void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

// called on keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos )
{
    // Handle keyboard input
    // end program
    if(key == 27) // esc
    {
      quitCall = true;
      glutIdleFunc(update);
      exit(0);
    }
    // continue program
  // redraw screen 
  glutPostRedisplay();    
}

// initialize basic geometry and shaders for this example
bool initialize()
{
    // define model with model loader
    bool result;

    // loads shaders to program
    programLoad.loadShader( vsFileName, fsFileName,  program );

    // now we set the locations of the attributes and uniforms
    // this allows us to access them easily while rendering

    // Get a handle for our "MVP" uniform
    loc_mvp = glGetUniformLocation(program, "mvpMatrix");
      if(loc_mvp == -1)
      {
        std::cerr << "[F] MVP NOT FOUND" << std::endl;
        return false;
      }       

    // Get a handle for our buffers
    loc_position = glGetAttribLocation(program, "v_position");
      if(loc_position == -1)
      {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
      }

    loc_color = glGetAttribLocation(program, "v_color");
      if(loc_color == -1)
      {
        std::cerr << "[F] COLOR NOT FOUND" << std::endl;
        return false;
      }      
    
    loc_uv = glGetAttribLocation(program, "v_uv");

    loc_normal = glGetAttribLocation(program, "v_normal");

    m_pMesh = new Mesh();
    result = m_pMesh -> loadMesh(objPtr);


      if( !result )
      {
        quitCall = true;
        glutIdleFunc(update);
        exit(0);
      }

    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(3.0f, 7.0f, -10.0f), //Eye Position
                        glm::vec3(0.0f, -0.2f, 0.0f), //Focus point
                        glm::vec3(0.0f, 1.0f, 0.0f)); //Positive Y is up

    projection = glm::perspective( 60.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

// delete old items
void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);   
}

// adds and removes menus
void manageMenus()
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
    // Clean up after ourselves
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
      quitCall = true;
      glutIdleFunc(update);
      break;
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
