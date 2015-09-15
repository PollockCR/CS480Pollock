#include "shader.h" // header file of shader loaders
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // makes passing matrices to shaders easier

//--Data types
// This object defines the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};

//--Global constants
const char* vsFileName = "../bin/shader.vs";
const char* fsFileName = "../bin/shader.fs";
const int NUM_SHADERS = 2;

// Global variables
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
ShaderLoader programLoad; // Load shader class

    // Initialize rotations
    bool rotateFlagPlanet = false;
    bool orbitFlagPlanet = false;
    bool rotateFlagMoon = false;
    bool orbitFlagMoon = false;

    // Paused rotations
    bool paused = false;

    // Quit call
    bool quitCall = false;

// uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

// attribute locations
GLint loc_position;
GLint loc_color;

// transform matrices
glm::mat4 model;// obj->world each object should have its own model matrix
glm::mat4 model_moon;
glm::mat4 view;// world->eye
glm::mat4 projection;// eye->clip
glm::mat4 mvp;// premultiplied modelviewprojection
glm::mat4 mvp_moon;

//--GLUT Callbacks
void render();
void update();
void updateMoon();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void menu(int id);
void start_stop_menu(int id);
void rotation_menu(int id);
void mouse(int button, int state, int x_pos, int y_pos);

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Main
int main(int argc, char **argv)
{
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);

    // Name and create the Window
    glutCreateWindow("Rotating Cube");

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Create menu

      // Create sub menu start-stop
      int startstop_menu = glutCreateMenu(start_stop_menu);
      glutAddMenuEntry("Start rotation of planet", 2);
      glutAddMenuEntry("Stop rotation of planet", 3);

      // Create sub menu rotate
      int rotate_menu = glutCreateMenu(rotation_menu);
      glutAddMenuEntry("Reverse rotation of planet", 4);
      glutAddMenuEntry("Reverse orbit of planet", 5); 
      glutAddMenuEntry("Reverse rotation of moon", 6);
      glutAddMenuEntry("Reverse orbit of moon", 7);      

    int main_menu = glutCreateMenu(menu); // Call menu function
    glutAddMenuEntry("Quit", 1);
    glutAddSubMenu("Start & stop rotations", startstop_menu);
    glutAddSubMenu("Reverse orbits & rotations", rotate_menu);
    glutAttachMenu(GLUT_RIGHT_BUTTON); //Called if there is a mouse click (right)

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutMouseFunc(mouse);// Called if there is a mouse click (left)

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    glutDestroyMenu(rotate_menu);
    glutDestroyMenu(startstop_menu);
    glutDestroyMenu(main_menu);
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //static float scale = 0.0f;

    //enable the shader program
    glUseProgram(program);


    // render first object

      //premultiply the matrix for this example
      mvp = projection * view * model;

      //upload the matrix to the shader
      glUniformMatrix4fv(loc_mvpmat, 2, GL_FALSE, glm::value_ptr(mvp));

      //set up the Vertex Buffer Object so it can be drawn
      glEnableVertexAttribArray(loc_position);
      glEnableVertexAttribArray(loc_color);
      glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);

        //set pointers into the vbo for each of the attributes(position and color)
        glVertexAttribPointer( loc_position,//location of attribute
                             3,//number of elements
                             GL_FLOAT,//type
                             GL_FALSE,//normalized?
                             sizeof(Vertex),//stride
                             0);//offset

        glVertexAttribPointer( loc_color,
                             3,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof(Vertex),
                             (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count
/*
    // render second object

      //premultiply the matrix for this example
      mvp_moon = projection * view * model_moon;

      //upload the matrix to the shader
      glUniformMatrix4fv(loc_mvpmat, 2, GL_FALSE, glm::value_ptr(mvp_moon));

      //set up the Vertex Buffer Object so it can be drawn
      glEnableVertexAttribArray(loc_position);
      glEnableVertexAttribArray(loc_color);
      glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);

        //set pointers into the vbo for each of the attributes(position and color)
        glVertexAttribPointer( loc_position,//location of attribute
                             3,//number of elements
                             GL_FLOAT,//type
                             GL_FALSE,//normalized?
                             sizeof(Vertex),//stride
                             0);//offset

        glVertexAttribPointer( loc_color,
                             3,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof(Vertex),
                             (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count

    // done rendering objects
*/
    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    float dt;

    // check for quit program
    if( quitCall )
    {
      exit(0);
    }

    // pause rotations
    else if( paused )
    {
      dt = 0.0;
    }

    else 
    {
      //total time
      dt = getDT();// if you have anything moving, use dt.
      
      static float angleRotatePlanet = 0.0;
      static float angleOrbitPlanet = 0.0;

      // check for reverse direction of rotation
      if( rotateFlagPlanet )
      {
        // reverse angle of planet
        angleRotatePlanet = angleRotatePlanet - (dt * M_PI/2); //move through -90 degrees a second

        // move in a circle
        model = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angleRotatePlanet), 0.0, 4.0 * cos(angleRotatePlanet)));
      }
      // normal direction
      else 
      {
        // update angle of planet
        angleRotatePlanet += dt * M_PI/2; //move through 90 degrees a second

        // move in a circle
        model = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angleRotatePlanet), 0.0, 4.0 * cos(angleRotatePlanet)));
      }

      // check for reverse direction of orbit
      if( orbitFlagPlanet )
      {
        // reverse angle of planet
        angleOrbitPlanet = angleOrbitPlanet - (dt * M_PI/2); //move through -90 degrees a second

        // rotate around y axis
        model = glm::rotate(model,angleOrbitPlanet,glm::vec3(0.0f,1.0f,0.0f));
      }
      // normal direction
      else 
      {
        // update angle of planet
        angleOrbitPlanet += dt * M_PI/2; //move through 90 degrees a second

        // rotate around y axis
        model = glm::rotate(model,angleOrbitPlanet,glm::vec3(0.0f,1.0f,0.0f));
      }

      updateMoon();

      // update the state of the scene
      glutPostRedisplay();//call the display callback
    }
}

void updateMoon()
{
/*
    //total time
    float dt = getDT();// if you have anything moving, use dt.

    float angleRotateMoon = 0.0;
    float angleOrbitMoon = 0.0;

    // check for reverse direction of rotation
    if( rotateFlagMoon )
    {
      // reverse angle of planet
      angleRotateMoon = angleRotateMoon - (dt * M_PI/2); //move through -90 degrees a second

      // move in a circle
      model_moon = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angleRotateMoon), 0.0, 4.0 * cos(angleRotateMoon)));
    }
    // normal direction
    else 
    {
      // update angle of planet
      angleRotateMoon += dt * M_PI/2; //move through 90 degrees a second

      // move in a circle
      model_moon = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angleRotateMoon), 0.0, 4.0 * cos(angleRotateMoon)));
    }

    // check for reverse direction of orbit
    if( orbitFlagMoon )
    {
      // reverse angle of planet
      angleOrbitMoon = angleOrbitMoon - (dt * M_PI/2); //move through -90 degrees a second

      // rotate around y axis
      model_moon = glm::rotate(model_moon,angleOrbitMoon,glm::vec3(0.0f,1.0f,0.0f));
    }
    // normal direction
    else 
    {
      // update angle of planet
      angleOrbitMoon += dt * M_PI/2; //move through 90 degrees a second

      // rotate around y axis
      model_moon = glm::rotate(model_moon,angleOrbitMoon,glm::vec3(0.0f,1.0f,0.0f));
    }
*/
}

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

void keyboard(unsigned char key, int x_pos, int y_pos )
{
    // Handle keyboard input
    // end program
    if(key == 27) // esc
    {
      quitCall = true;
      glutIdleFunc(update);
    }
    // rotate cube
    else
    {
      if( !rotateFlagPlanet ) 
      {
        rotateFlagPlanet = true;
      }     
      else
      {
        rotateFlagPlanet = false;
      }

      if( !orbitFlagPlanet ) 
      {
        orbitFlagPlanet = true;
      }     
      else
      {
        orbitFlagPlanet = false;
      }
    }
}

bool initialize()
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    Vertex geometry[] = { {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},

                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}}
                        };
    // create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

    // loads shaders to program
    programLoad.loadShader( vsFileName, fsFileName,  program );

    // now we set the locations of the attributes and uniforms
    // this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
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
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}

// menu choices 
void menu(int id)
{
  // switch case for menu options
  switch(id)
  {
    // exit the program
    case 1:
      quitCall = true;
      glutIdleFunc(update);
      break;

    // call the rotation menu function
    case 2:
    case 3:
      start_stop_menu(id);
      break;

    default:
      rotation_menu(id);
      break;
  }
  // redraw screen without menu
  glutPostRedisplay();
}

// menu choices 
void start_stop_menu(int id)
{
  // switch case for menu options
  switch(id)
  {
    // update display, start rotation
    case 2:
      paused = false;
      glutIdleFunc(update);
      //glutPostRedisplay();
      break;
    // stop rotation
    case 3:
      paused = true;
      glutIdleFunc(update);
      //glutPostRedisplay();
      break;
  }
  // redraw screen without menu
  glutPostRedisplay();
}

// menu choices 
void rotation_menu(int id)
{
  // switch case for menu options
  switch(id)
  {
    // reverse rotation of planet
    case 4:
      rotateFlagPlanet = !rotateFlagPlanet;
      break;
    // reverse orbit of planet
    case 5:
      orbitFlagPlanet = !orbitFlagPlanet;
      break;
    // reverse rotation of moon
    case 6:
      rotateFlagMoon = !rotateFlagMoon;
      break;
    // reverse orbit of moon
    case 7:
      orbitFlagMoon = !orbitFlagMoon;
      break;      
  }
  // redraw screen without menu
  glutPostRedisplay();
}

// actions for left mouse click
void mouse(int button, int state, int x_pos, int y_pos)
{
  // check for correct button  (left click)
  if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    // change rotation flag
    if( !rotateFlagPlanet ) 
    {
      rotateFlagPlanet = true;
    }     
    else
    {
      rotateFlagPlanet = false;
    }

    // change orbit flag
    if( !orbitFlagPlanet ) 
    {
      orbitFlagPlanet = true;
    }     
    else
    {
      orbitFlagPlanet = false;
    }
  }

}