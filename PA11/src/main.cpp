// removed idle function, not in use
// created seperate files for fragment and vertex shader
#include "shader.h" // header file of shader loaders
#include "mesh.h" // header file of object loader
#include "loadImage.h" // header file for image class
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <cstring>
#include <vector>
#include <math.h>
#include <ctime>

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

// Bullet library
#include <btBulletDynamicsCommon.h>

// DATA TYPES

// GLOBAL CONSTANTS
const char* vsFileName = "../bin/shader.vs";
const char* fsFileName1 = "../bin/shader1.fs";
const char* fsFileName2 = "../bin/shader2.fs";
const char* fsFileName3 = "../bin/shader3.fs";
const char* fsFileName4 = "../bin/shader4.fs";
const char* defaultInfo = "../bin/imageInfo.txt";
const char* blankTexture = "../../Resources/white.png";

// GLOBAL VARIABLES

  // Window size
  int w = 1280, h = 768;

  // geomerty size
  int geometrySize;

  // The GLSL program handle
  GLuint program;
  //GLuint vbo_geometry;
  //GLuint normalbuffer; // Normal Buffer
  //GLuint texture;

  // rotations
  int orbit = -1;
  int rotation = -1;

  // uniform locations
  GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader
  GLint viewMatrixID;
  GLint modelMatrixID;

  // attribute locations
  GLint loc_position;
  GLint loc_texture;
  GLint vertexNormal_modelspaceID;

  // transform matrices
  glm::mat4 view;// world->eye
  glm::mat4 projection;// eye->clip
  glm::mat4 mvp;// premultiplied modelviewprojection

  // Images
  std::vector<Image> images;
  int numImages = 0;
  int viewType = 3;

  // time information
  std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2, start;
  float duration;
  bool timeflag = true;

// FUNCTION PROTOTYPES

  //--GLUT Callbacks
  void render();

  // update display functions
  void update();
  void reshape(int n_w, int n_h);

  // called upon input
  void keyboard(unsigned char key, int x_pos, int y_pos);
  void keyboardUP(unsigned char key, int x_pos, int y_pos );
  void manageMenus(bool quitCall);
  void menu(int id);
  void mouse(int button, int state, int x_pos, int y_pos);

  //--Resource management
  bool initialize( const char* filename);
  void cleanUp();

  //--Time function
  float getDT();

  //Load Image info
  bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes, int numOfImages );

  void sPrint( float xPos, float yPos, const char *str, int fontSize);

  //Bullet 
  btDiscreteDynamicsWorld *dynamicsWorld;
  btRigidBody *rigidBodySphere;
  btRigidBody *rigidBodyCube;
  btRigidBody *rigidBodyCylinder;
  btTriangleMesh *trimesh = new btTriangleMesh();
  btTransform startPos(btQuaternion(0, 0, 0, 1), btVector3(2, 1, 0));
  
  //Bullet collisions
  #define BIT(x) (1<<(x))
  enum collisionObject 
  {
    ball = BIT(0), 
    wall = BIT(1), 
    win = BIT(2), 
  };

  int ballBouncesOff = wall | win;
  int wallDeflects = ball;
  
  //pan
  void pan();
  

  //directions
  bool forward = false;
  bool backward = false;
  bool goLeft = false;
  bool goRight = false;
  double rotationDegrees = 0.0;
  bool cylforward = false;
  bool cylbackward = false;
  bool cylgoLeft = false;
  bool cylgoRight = false;
  
  //current view position
  static float* source = new float[3];
  static float* dest = new float[3];
  
  // Update view flag
  bool updateViewFlag = false;
  
  //need gravity vectors to change as board moves
  btVector3 earthGravity = btVector3(0.0,-10.0,0.0);
  btVector3 gravityAwayViewer = btVector3(0.0,-10.0,10.0);
  btVector3 gravityTowardViewer = btVector3(0.0,-10.0,-10.0);
  btVector3 gravityLeftViewer = btVector3(10.0,-10.0,0.0);
  btVector3 gravityRightViewer = btVector3(-10.0,-10.0,0.0);
  btVector3 gravityTowardLeft = btVector3(10.0,-10.0,-10.0);
  btVector3 gravityTowardRight = btVector3(-10.0,-10.0,-10.0);
  btVector3 gravityAwayLeft = btVector3(10.0,-10.0,10.0);
  btVector3 gravityAwayRight = btVector3(-10.0,-10.0,10.0);
  
  //rotation directions
  float xRotation = 0.0f;
  float zRotation = 0.0f;
  float xTilt = 0.0f;
  float zTilt = 0.0f;
  
  //score
  int score;
  int highScore;




// MAIN FUNCTION
int main(int argc, char **argv)
{
    bool init = false;

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    // Name and create the Window
    glutCreateWindow("Labyrinth");

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
    glutKeyboardUpFunc(keyboardUP);
    glutMouseFunc(mouse);//Called if there is mouse input

    // add menus
    manageMenus( false );
    
    // Initialize all of our resources(shaders, geometry)
    // pass default planet info if not given one 
    if( argc != 3 )
    {
      init = initialize( defaultInfo );
    }
    // or, pass planet info given from command line argument
    else
    {
      init = initialize( argv[1] );
    }

//////////////////////////////////////////////////////////////////////////
    //create brodphase
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();

    //create collision configuration
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

    //create a dispatcher
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

    //create a solver
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    //create the physics world
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

     //set the gravity
     dynamicsWorld->setGravity(btVector3(0, -10, 0));

    /*//create a game board which the objects will be on
    btCollisionShape* ground = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    btCollisionShape* wallOne = new btStaticPlaneShape(btVector3(-1, 0, 0), 1);
    btCollisionShape* wallTwo = new btStaticPlaneShape(btVector3(1, 0, 0), 1);
    btCollisionShape* wallThree = new btStaticPlaneShape(btVector3(0, 0, 1), 1);
    btCollisionShape* wallFour = new btStaticPlaneShape(btVector3(0, 0, -1), 1);*/
    
    // now we can make the maze in one line
     btBvhTriangleMeshShape* mazeShape = new btBvhTriangleMeshShape(trimesh, false);

    //create sphere and set radius to 1
    btCollisionShape* sphere = new btSphereShape(.3);


/*----------------------this is the gameboard--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the ground
    btDefaultMotionState* groundMotionState = NULL;
    groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    //here we construct the ground using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, mazeShape, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(groundRigidBody, wall, wallDeflects );
/*-----------------------------------------------------------------------------*/


/*----------------------this is the sphere--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the sphere
    btDefaultMotionState* sphereMotionState = NULL;
    sphereMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(2, 1, 0)));

    // the sphere must have a mass
    btScalar mass = 50;

    //we need the inertia of the sphere and we need to calculate it
    btVector3 sphereInertia(0, 0, 0);
    sphere->calculateLocalInertia(mass, sphereInertia);

    //Here we construct the sphere with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(mass, sphereMotionState, sphere, sphereInertia);
    rigidBodySphere = new btRigidBody(sphereRigidBodyCI);
    rigidBodySphere->setActivationState(DISABLE_DEACTIVATION);
    rigidBodySphere->setFriction(50);
    rigidBodySphere->setRestitution(0);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodySphere, ball, ballBouncesOff);
/*-----------------------------------------------------------------------------*/
        
    // if initialized, begin glut main loop
    if(init)
    {
        start = t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // remove menus
    manageMenus( true );

    //////////// clean up and end program
    // delete the pointers
    dynamicsWorld->removeRigidBody(groundRigidBody);
    delete groundRigidBody->getMotionState();
    delete groundRigidBody;
        
    dynamicsWorld->removeRigidBody(rigidBodySphere);
    delete rigidBodySphere->getMotionState();
    delete rigidBodySphere;
    delete sphere;
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;

    cleanUp();
    return 0;
}

// FUNCTION IMPLEMENTATION



// render the scene
void render()
{
  // clear the screen
  glClearColor(0.3, 0.3, 0.3, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);

    sPrint(-0.95,0.9,(char*)"WASD to Move Table", 12);
    sPrint(-0.95,0.8,(char*)"IJKL to Pan", 12);
    sPrint(-0.95,0.7,(char*)"1234 to Change Lighting", 12);
    std::string viewText;
    if( viewType == 1 )
    {
      viewText = "Light source: Ambient";
    }
    else if( viewType == 2 )
    {
      viewText = "Light source: Spotlight";
    }
    else if( viewType == 3 )
    {
      viewText = "Light source: Point";
    }
    else if( viewType == 4 )
    {
      viewText = "Light source: Distance";
    }    
    sPrint(-0.95,0.6,viewText.c_str(), 12);
    sPrint(-0.95,0.5,(char*)"Spacebar to Pause/Resume", 12);
    sPrint(-0.95,0.4,(char*)"Right Click for More Options", 12);      
    sPrint(-0.95,0.3,(char*)"H to Hide Menu", 12);
    sPrint(-0.95,0.2,(char*)"Esc to Quit", 12);
    std::string viewScore;
    viewScore = "High Score: " + std::to_string(highScore);
    sPrint(-0.95,0.1,viewScore.c_str(), 12);
    std::string timerText;
    duration = duration + (getDT()*10);
    timerText = "Time Elapsed: " + std::to_string(duration);
    sPrint(-0.95,-0.9,timerText.c_str(), 18);

  // enable the shader program
  glUseProgram(program);
  GLuint lightID = glGetUniformLocation(program, "LightPosition_worldspace");

    if( viewType == 2 || viewType == 3 )
    {
      // light
      glm::vec3 lightPos = glm::vec3(0,4,0);
      glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);
    }
    else if( viewType == 4 )
    {
      glm::vec3 lightPos = glm::vec3(0,1,-5);
      glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);
    }

 // loop through each planet
    for( int index = 0; index < numImages; index++ )
    {
      // premultiply the matrix for this example
      images[index].mvp = projection * view * images[index].model;

      // upload the matrix to the shader
      glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, &(images[index].mvp[0][0])); 
      glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &(images[index].model[0][0]));
      glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &(images[index].view[0][0]));    

      // set up the Vertex Buffer Object so it can be drawn
      glEnableVertexAttribArray(loc_position);
      glBindBuffer(GL_ARRAY_BUFFER, images[index].vbo_geometry);

      // set pointers into the vbo for each of the attributes(position and color)
      glVertexAttribPointer( loc_position,//location of attribute
                             3,//number of elements
                             GL_FLOAT,//type
                             GL_FALSE,//normalized?
                             sizeof(Vertex),//stride
                             0);//offset

      glEnableVertexAttribArray(loc_texture);
      glBindTexture(GL_TEXTURE_2D, images[index].texture);

      glVertexAttribPointer( loc_texture,
                             2,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof(Vertex),
                             (void*)offsetof(Vertex,uv));


      glEnableVertexAttribArray(vertexNormal_modelspaceID);
      glBindBuffer(GL_ARRAY_BUFFER, images[index].normalbuffer);

        // 3rd attribute buffer : normals  
        glVertexAttribPointer( vertexNormal_modelspaceID,
                               3,
                               GL_FLOAT,
                               GL_FALSE,
                               sizeof(Vertex),
                               (void*)offsetof(Vertex,normals));       

      glDrawArrays(GL_TRIANGLES, 0, images[index].geometrySize);//mode, starting index, count
    }

  //clean up
  glDisableVertexAttribArray(loc_position);
  glDisableVertexAttribArray(loc_texture);
  glDisableVertexAttribArray(vertexNormal_modelspaceID);   
               
  //swap the buffers
  glutSwapBuffers();

}

// called on idle to update display
void update()
{
  if( timeflag )
  {
  // update object
    float dt = getDT();
    
    // add the forces to the sphere for movement
    if(forward && !backward && !goLeft && !goRight)
    {
        zRotation = (1 - 0.03f) * zRotation + 0.0f*0.03;
        dynamicsWorld->setGravity(gravityAwayViewer);
        //forward = false;
    }
    else if(backward && !forward && !goLeft && !goRight)
    {
        zRotation = (1 - 0.03f)*zRotation + 0.0f*0.03;
        dynamicsWorld->setGravity(gravityTowardViewer);
        //backward = false;
    }
    else if(goLeft && !backward && !forward && !goRight)
    {
        xRotation = (1 - 0.03f)*xRotation + 0.0f*0.03;
        dynamicsWorld->setGravity(gravityLeftViewer);
        //goLeft = false;
    }
    else if(goRight && !goLeft && !backward && !forward)
    {
        xRotation = (1 - 0.03f)*xRotation + 0.0f*0.03;
        dynamicsWorld->setGravity(gravityRightViewer);
        //goRight = false;
    }
    else if(goRight && !goLeft && !backward && forward)
    {
        dynamicsWorld->setGravity(gravityAwayRight);
        //goRight = false;
    }
    else if(!goRight && goLeft && !backward && forward)
    {
        dynamicsWorld->setGravity(gravityAwayLeft);
        //goRight = false;
    }
    else if(goRight && !goLeft && backward && !forward)
    {
        dynamicsWorld->setGravity(gravityTowardRight);
        //goRight = false;
    }
    else if(!goRight && goLeft && backward && !forward)
    {
        dynamicsWorld->setGravity(gravityTowardLeft);
        //goRight = false;
    }
    else 
    {
        dynamicsWorld->setGravity(earthGravity);
        xRotation = (1 - 0.03f)*xRotation + 0.0f*0.03;
        zRotation = (1 - 0.03f)*zRotation + 0.0f*0.03;
    }

    //find angle of tilt
    xTilt = xRotation * 100.0 * M_PI / 180.0;
    zTilt = zRotation * 100.0 * M_PI / 180.0;

    // tilt mazew 
    images[0].model = glm::rotate(glm::mat4(1.0f), zTilt, glm::vec3(1.0f, 0.0f, 0.0f));
    images[0].model = glm::rotate(images[0].model, -xTilt, glm::vec3(0.0f, 0.0f, 1.0f));
    
    dynamicsWorld->stepSimulation(dt, 10);


    btTransform trans;

    btScalar m[16];

    //set the sphere to it's respective model
    rigidBodySphere->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(m);
    images[1].model = glm::make_mat4(m);
    glm::vec4 spherePos = images[1].model * glm::vec4(1.0f);

    //give ball same tilt as table
    images[1].model = glm::rotate(images[1].model, zTilt, glm::vec3(1.0f, 0.0f, 0.0f));
    images[1].model = glm::rotate(images[1].model, -xTilt, glm::vec3(0.0f, 0.0f, 1.0f));

    if (spherePos.y < -20.0)
    {
        rigidBodySphere->setWorldTransform(startPos);
        rigidBodySphere->setLinearVelocity(btVector3(0.0,0.0,0.0));
        score++;
        highScore = score;    

    }
  }

  // update the state of the scene
  glutPostRedisplay();//call the display callback

  // clean up!
  rigidBodySphere->clearForces();
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


// reset keys
void keyboardUP(unsigned char key, int x_pos, int y_pos )
{

    if((key == 'w')||(key == 'W'))
    {
      forward = false;
    }
    if((key == 'a')||(key == 'A'))
    {
      goLeft = false;
    }
    if((key == 's')||(key == 'S'))
    {
      backward = false;
    }
    if((key == 'd')||(key == 'D'))
    {
      goRight = false;
    }

}



// called on keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos )
{
    ShaderLoader programLoad;
    float tiltBoard = 0.02;
    
  // Handle keyboard input - end program
    if((key == 27)||(key == 'q')||(key == 'Q'))
    {
      glutLeaveMainLoop();
    }
    else if((key == 'w')||(key == 'W'))
    {
      zRotation += tiltBoard;
      if (zRotation > 1.0f)
        {
            zRotation = 1.0f;
        }
      forward = true;
    }
    else if((key == 'a')||(key == 'A'))
    {
      xRotation += tiltBoard;
      if (xRotation > 1.0f)
        {
            xRotation = 1.0f;
        }
      goLeft = true;
    }
    else if((key == 's')||(key == 'S'))
    {
      zRotation -= tiltBoard;
      backward = true;
    }
    else if((key == 'd')||(key == 'D'))
    {
      xRotation -= tiltBoard;
      goRight = true;
    }
    else if(key == '1')
    {
      programLoad.loadShader( vsFileName, fsFileName1, program );
      viewType = 1;
    }         
    else if(key == '2')
    {
      programLoad.loadShader( vsFileName, fsFileName2, program );
      viewType = 2;
    }  
    else if(key == '3')
    {
      programLoad.loadShader( vsFileName, fsFileName3, program );
      viewType = 3;
    }  
    else if(key == '4')
    {
      programLoad.loadShader( vsFileName, fsFileName4, program );
      viewType = 4;
    }   
    else if(key == ' ')
    {
      timeflag = !timeflag;
    }
    
    // pan
    if((key == 'i')||(key == 'I'))
    {
      dest[0] = 0.0;
      dest[1] = 24.0;
      dest[2] = 18.0;
      updateViewFlag = true;        
      pan();
    }
    if((key == 'j')||(key == 'J'))
    {
      dest[0] = 18.0;
      dest[1] = 24.0;
      dest[2] = 0.0; 
      updateViewFlag = true;        
      pan();
    }
    if((key == 'k')||(key == 'K'))
    {
      dest[0] = 0.0;
      dest[1] = 24.0;
      dest[2] = -18.0; 
      updateViewFlag = true;        
      pan();
    }
    if((key == 'l')||(key == 'L'))
    {
      dest[0] = -18.0;
      dest[1] = 24.0;
      dest[2] = 0.0; 
      updateViewFlag = true;        
      pan(); 
    }   
}

// initialize basic geometry and shaders for this example
bool initialize( const char* filename)
{
    // define model with model loader
    /*bool geometryLoadedCorrectly;
    Mesh object;
    ShaderLoader programLoad;*/
    int index;
    std::vector<Mesh> meshes;
    ShaderLoader programLoad;

    // load the image info
    if( !loadInfo( filename, meshes, numImages ) )
    {
      return false;
    }


     // loop through each planet
      for( index = 0; index < numImages; index++ )
      {
        // Create a Vertex Buffer object to store this vertex info on the GPU
        glGenBuffers(1, &(images[index].vbo_geometry));
        glBindBuffer(GL_ARRAY_BUFFER, images[index].vbo_geometry);
        glBufferData(GL_ARRAY_BUFFER, meshes[index].geometry.size()* sizeof(Vertex), &(meshes[index].geometry[0]), GL_STATIC_DRAW);

        // Create Texture object
        glGenTextures(1, &(images[index].texture));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, images[index].texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, images[index].imageCols, images[index].imageRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, images[index].m_blob.data());
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      

        glGenBuffers(1, &(images[index].normalbuffer));
        glBindBuffer(GL_ARRAY_BUFFER, images[index].normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, meshes[index].geometry.size() * sizeof(Vertex), &(meshes[index].geometry[0]), GL_STATIC_DRAW);    
        //glBufferData(GL_ARRAY_BUFFER, meshes[index].geometry.size() * sizeof(Vertex), &(meshes[index].geometry[offsetof(Vertex,normals)]), GL_STATIC_DRAW);    

      }      

        // loads shaders to program
        programLoad.loadShader( vsFileName, fsFileName3, program );

    // Get a handle for our "MVP" uniform
    loc_mvpmat = glGetUniformLocation(program, "mvpMatrix");
      if(loc_mvpmat == -1)
      {
        std::cerr << "[F] MVP MATRIX NOT FOUND" << std::endl;
        return false;
      }       

    viewMatrixID = glGetUniformLocation(program, "V");
      if(viewMatrixID == -1)
      {
        std::cerr << "[F] VIEW NOT FOUND" << std::endl;
        return false;
      }  

    modelMatrixID = glGetUniformLocation(program, "M");
      if(modelMatrixID == -1)
      {
        std::cerr << "[F] MODEL NOT FOUND" << std::endl;
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
        std::cerr << "[F] UV NOT FOUND" << std::endl;
        return false;
      }

    vertexNormal_modelspaceID = glGetAttribLocation(program, "vertexNormal_modelspace");
      if(vertexNormal_modelspaceID == -1)
      {
        std::cerr << "[F] NORMAL NOT FOUND" << std::endl;
        return false;
      }


    // set view variables
    source[0] = 0.0;
    source[1] = 24.0;
    source[2] = -3.0;  
    dest[0] = 0.0;
    dest[1] = 24.0;
    dest[2] = -3.0;


    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(source[0], source[1], source[2]), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane
                                   
    
    images[numImages].model = glm::scale(images[numImages].model, glm::vec3(25, 25, 25));

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

// load info from file into Images
bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes, int numOfImages )
{
  // initialize variables
  std::ifstream ifs(infoFilepath, std::ifstream::in);
  bool geometryLoadedCorrectly = false;
  bool imageLoadedCorrectly = false;
  int index = 0;
  int numOfMeshes = 0;

  // check for open file
  if( !ifs.is_open() )
  {
    std::cerr << "[F] FAILED TO READ FILE!" << infoFilepath << std::endl;
    return false;    
  }

  // read in number of Images
  ifs >> numOfImages;

  // check for invalid file format
  if( numOfImages == -1 )
  {
    std::cerr << "[F] FAILED TO READ FILE! INVALID FORMAT" << std::endl;
    return false; 
  }

  // loop through each image
  for( index = 0; index < numOfImages; index++ )
  {
    // create new Image
    Image tempImage;
    images.push_back(tempImage);

    // create new mesh
    Mesh tempMesh;
    meshes.push_back(tempMesh);

    // read from file

      // load obj file
      std::string objFilepath;
      ifs >> objFilepath;
      geometryLoadedCorrectly = meshes[index].loadMesh( objFilepath.c_str(), numOfMeshes);

        // return false if not loaded
        if( !geometryLoadedCorrectly )
        {
          std::cerr << "[F] GEOMETRY NOT LOADED CORRECTLY" << std::endl;
          return false;
        }
        
       // set number of vertices
       unsigned int numOfVertices = meshes[index].geometry.size(); 

       //check if we are reading in the maze 
       if (index == 0)
        {
            //load trimesh for maze
            for ( int index2 = 0; index2 < numOfMeshes; index2++)
            {
                for (unsigned int index3 = 0; index3 < numOfVertices; index3 += 3)
                {
                trimesh->addTriangle(btVector3(meshes[index2].geometry[index3].position[0], 
                                               meshes[index2].geometry[index3].position[1], 
                                               meshes[index2].geometry[index3].position[2]), 
                                           
                                     btVector3(meshes[index2].geometry[index3+1].position[0], 
                                               meshes[index2].geometry[index3+1].position[1], 
                                               meshes[index2].geometry[index3+1].position[2]), 
                                            
                                     btVector3(meshes[index2].geometry[index3+2].position[0], 
                                               meshes[index2].geometry[index3+2].position[1], 
                                               meshes[index2].geometry[index3+2].position[2]), false);  
                }
            }
        }

      // load texture
      std::string textureFilepath;
      ifs >> textureFilepath;
      imageLoadedCorrectly = images[index].loadImage(textureFilepath.c_str());

        // return false if not loaded
        if( !imageLoadedCorrectly )
        {
          return false;
        }

      // save size of geometry
      images[index].geometrySize = meshes[index].geometry.size();
    
  }

  // update image count
  numImages = images.size();       


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

    // clean up each planet
    for( index = 0; index < numImages; index++ )
    {
      glDeleteBuffers(1, &(images[index].vbo_geometry));
      glDeleteBuffers(1, &(images[index].texture));
      glDeleteBuffers(1, &(images[index].normalbuffer));        
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
    glutAddMenuEntry("Pause/Resume Game", 1);
    glutAddMenuEntry("Exit Program", 2);;

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
    case 1:
      timeflag = !timeflag;
      break;
    // call the rotation menu function
    case 2:
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
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
}

//returns the time delta
float getDT()
{
    float ret;

    // update time using time elapsed since last call
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();

    // check if paused
    if( !timeflag )
    {
      return 0.0;
    }

    return ret;
}

// prints a string to the screen
void sPrint( float xPos, float yPos, const char *str, int fontSize)
{
  int length;
  int index;

  // see how many characters are in text string.
  length = strlen( str ); 

  // location to start printing text
  glRasterPos2f(xPos, yPos); 

  if( fontSize <= 12 )
  {
    // loop for small text
    // until index is greater then length
    for( index = 0; index < length; index++) 
    {
      // Print a character on the screen
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[index]); 
    }
  }
  else
  {
    // loop for large text
    // until index is greater then length
    for( index = 0; index < length; index++) 
    {
      // Print a character on the screen
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[index]); 
    }  
  }
}


void pan()
{
    // pre pan movement
      // make sure we want to move
      if (updateViewFlag)
      {

        //paused = true;
        // check source vs dest coords
        // increment accordingly
        for (int i = 0; i < 3; i++)
        {

            if (i < 3)
            {
              if (source[i] < dest[i]) 
                  source[i] += 0.18;
              if (source[i] > dest[i]) 
                  source[i] -= 0.18; 
                glutPostRedisplay(); 
            }
            else
            {
              if (source[i] < dest[i]) 
                  source[i] += 0.18;
              if (source[i] > dest[i]) 
                  source[i] -= 0.18; 
                glutPostRedisplay(); 
            }
        }
          
        // check acceptable ranges
        if (((dest[0] - source[0] <= 0.5) && (dest[0] - source[0] >= -0.5))&&
            ((dest[1] - source[1] <= 0.5) && (dest[1] - source[1] >= -0.5))&&
            ((dest[2] - source[2] <= 0.5) && (dest[2] - source[2] >= -0.5)))
        {
          // done with pan
          updateViewFlag = false;

          // update view to dest
          view = glm::lookAt( glm::vec3(dest[0], dest[1], dest[2]), //Eye Position
                  glm::vec3(0.0, 0.0, 0.0), //Focus point
                  glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

        }
        else
        {
          // update to incremented view
          view = glm::lookAt( glm::vec3(source[0], source[1], source[2]), //Eye Position
                glm::vec3(0.0, 0.0, 0.0), //Focus point
                glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
          glutPostRedisplay();  
        } 
      }
      
      else
      {
        // keep source data current
        for (int i = 0; i < 3; i++)
        {
            if (source[i] < dest[i]) 
                source[i] += 0.35;
            if (source[i] > dest[i]) 
                source[i] -= 0.35;
        }
        // locked view
        view = glm::lookAt( glm::vec3(dest[0], dest[1], dest[2]), //Eye Position
                glm::vec3(0.0, 0.0, 0.0), //Focus point
                glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
      }
    //}
}








