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
#include <stdlib.h>

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
const char* fsFileName = "../bin/shader.fs";
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
  glm::mat4 view;// world->eye
  glm::mat4 projection;// eye->clip
  glm::mat4 mvp;// premultiplied modelviewprojection

  // Images
  std::vector<Image> images;
  int numImages = 0;

  // Time Flag
  bool started = false;
  bool timeflag = false;

  // Menu Flag
  bool menuflag = false;

  // New game flag
  bool newGame = false;

  // Update view flag
  bool updateViewFlag = false;


  // time information
  std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;

  #define BIT(x) (1<<(x))
  enum collisionObject 
  {
    paddle = BIT(0), 
    wall = BIT(1), 
    barrier = BIT(2), 
    PUCK = BIT(3), 
    TABLE = BIT(4),
  };

  int puckBouncesOff = wall | paddle | PUCK;
  int paddleBouncesOff = wall | paddle | PUCK | barrier;
  int wallDeflects = paddle | PUCK;
  int barrierDeflects = paddle;
  int tableDeflects = paddle | PUCK;

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

  //mouse stuff
  bool mouseCanMove = false;///
  bool mouseOn = true;
  int mouseXAxis, mouseYAxis;///

  //ai stuff
  bool aiCanMove = false;
  int aiXAxis, aiYAxis;
  bool level1 = true;
  bool level2 = false;
  bool level3 = false; 
  bool keyTPressed = false;

  //scores
  int t1score = 0;
  int t2score = 0;
  bool justScored = false;
  int timer = 0;

  // current view positions
  static float* source = new float[3];
  static float* dest = new float[3];

  // current point of view
  bool player1POV = false;
  bool player2POV = false;
  bool leftSidePOV = false;
  bool rightSidePOV = false;

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
  void subMenu2 (int num);
  void subMenu(int num);
  void mainMenu(int num);
  void mouse(int button, int state, int x_pos, int y_pos);
  void arrowKeys(int button, int x_pos, int y_pos);
  void arrowKeysUp(int button, int x_pos, int y_pos);
  void moveMouse (int x, int y);
  void moveAI (int x, int y);

  //--Resource management
  bool initialize( const char* filename);
  void cleanUp();

  //--Time function
  float getDT();

  //Load Image info
  bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes, int numOfImages );

  void sPrint( float xPos, float yPos, const char *str, int fontSize);
  void pan();

  //Bullet 
  btDiscreteDynamicsWorld *dynamicsWorld;
  btRigidBody *rigidBodySphere;
  btRigidBody *rigidBodyCylinder;
  btRigidBody *rigidBodyPuck;///
  btTransform puckStart;




// MAIN FUNCTION
int main(int argc, char **argv)
{
  bool init = false;

  // Initialize glut
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(w, h);
  
  // Name and create the Window
  glutCreateWindow("Welcome to Air Hockey");

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
  glutPassiveMotionFunc(moveMouse);
  //glutMouseFunc(mouse);//Called if there is mouse input
  glutSpecialFunc(arrowKeys);
  glutKeyboardUpFunc(keyboardUP);
  glutSpecialUpFunc(arrowKeysUp);

	srand(getDT());

  // add menus
  manageMenus( false );

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
     dynamicsWorld->setGravity(btVector3(0, -9.85, 0));

    //create a game board which the objects will be on
    btCollisionShape* ground = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    btCollisionShape* wallOne = new btStaticPlaneShape(btVector3(-1, 0, 0), 1);
    btCollisionShape* wallTwo = new btStaticPlaneShape(btVector3(1, 0, 0), 1);
    btCollisionShape* wallThree = new btStaticPlaneShape(btVector3(0, 0, 1), 1);
    btCollisionShape* wallFour = new btStaticPlaneShape(btVector3(0, 0, -1), 1);

    //create paddlePlayer1 and set radius to 1
    btCollisionShape* paddlePlayer1 = new btCylinderShape(btVector3(1.0,0.3,1.0));

    //create a paddlePlayer2 and set radius of each axis to 1.0
    btCollisionShape* paddlePlayer2 = new btCylinderShape(btVector3(1.0,0.3,1.0));

    // create a middle barrier for the paddles
    btCollisionShape* middleBarrier = new btBoxShape(btVector3(8,8,0.0001));

    // create a puck
    btCollisionShape* puck = new btCylinderShape(btVector3(1.0,0.3,1.0));
   


/*----------------------this is the gameboard--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the ground
    btDefaultMotionState* groundMotionState = NULL;
    groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 2, 0)));
    //here we construct the ground using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, ground, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(groundRigidBody, TABLE, tableDeflects);
        
    ////make the first wall
    btDefaultMotionState* wallOneMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(7.0, 0, 0)));
    //here we construct the first wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallOneRigidBodyCI(0, wallOneMotionState, wallOne, btVector3(0, 0, 0));
    btRigidBody* wallOneRigidBody = new btRigidBody(wallOneRigidBodyCI);
    wallOneRigidBody->setRestitution(0.9f);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallOneRigidBody, wall, wallDeflects );


    ////make the second wall
    btDefaultMotionState* wallTwoMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-7.0, 0, 0)));
    //here we construct the second wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallTwoRigidBodyCI(0, wallTwoMotionState, wallTwo, btVector3(0, 0, 0));
    btRigidBody* wallTwoRigidBody = new btRigidBody(wallTwoRigidBodyCI);
    wallTwoRigidBody->setRestitution(0.9f);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallTwoRigidBody, wall, wallDeflects);


    ////make the third wall FRONTBACK
    btDefaultMotionState* wallThreeMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -13.0)));
    //here we construct the third wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallThreeRigidBodyCI(0, wallThreeMotionState, wallThree, btVector3(0, 0, 0));
    btRigidBody* wallThreeRigidBody = new btRigidBody(wallThreeRigidBodyCI);
    wallThreeRigidBody->setRestitution(0.9f);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallThreeRigidBody, wall, wallDeflects);


    ////make the fouth wall FRONTBACK
    btDefaultMotionState* wallFourMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 13.0)));
    //here we construct the fourth wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallFourRigidBodyCI(0, wallFourMotionState, wallFour, btVector3(0, 0, 0));
    btRigidBody* wallFourRigidBody = new btRigidBody(wallFourRigidBodyCI);
    wallFourRigidBody->setRestitution(0.9f);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallFourRigidBody, wall, wallDeflects);
/*-----------------------------------------------------------------------------*/


/*----------------------this is the paddlePlayer1--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the paddlePlayer1
    btDefaultMotionState* sphereMotionState = NULL;
    sphereMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -6)));

    // the paddlePlayer1 must have a mass
    btScalar mass = 100;

    //we need the inertia of the paddlePlayer1 and we need to calculate it
    btVector3 sphereInertia(0, 10, 15);
    paddlePlayer1->calculateLocalInertia(mass, sphereInertia);

    //Here we construct the paddlePlayer1 with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(mass, sphereMotionState, paddlePlayer1, sphereInertia);
    rigidBodySphere = new btRigidBody(sphereRigidBodyCI);
    rigidBodySphere->setActivationState(DISABLE_DEACTIVATION);
    rigidBodySphere->setFriction(0.0);
    rigidBodySphere->setLinearFactor(btVector3(1,0,1));
    rigidBodySphere->setAngularFactor(btVector3(0,1,0));

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodySphere, paddle, paddleBouncesOff);
/*-----------------------------------------------------------------------------*/
        
/*----------------------this is the paddlePlayer2--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the paddlePlayer2
    btDefaultMotionState* cylinderMotionState = NULL;
    cylinderMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 6)));

    //the cylinder must have a mass
    mass = 100;

    //we need the inertia of the cylinder and we need to calculate it
    btVector3 cylinderInertia(0, 10, 15);
    paddlePlayer2->calculateLocalInertia(mass, cylinderInertia);

    //Here we construct the cylinder with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo cylinderRigidBodyCI(mass, cylinderMotionState, paddlePlayer2, cylinderInertia);
    rigidBodyCylinder = new btRigidBody(cylinderRigidBodyCI);
    rigidBodyCylinder->setActivationState(DISABLE_DEACTIVATION);
    rigidBodyCylinder->setFriction(0.0);
    rigidBodyCylinder->setLinearFactor(btVector3(1,0,1));
    rigidBodyCylinder->setAngularFactor(btVector3(0,1,0));

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodyCylinder, paddle, paddleBouncesOff);
/*-----------------------------------------------------------------------------*/


/*----------------------this is the puck--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the puck
    btDefaultMotionState* puckMotionState = NULL;
    puckMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

    //the puck must have a mass
    // we make the mass less so it moves 
    // faster than the paddles
    mass = 5;

    //we need the inertia of the puck and we need to calculate it
    btVector3 puckInertia(0, 0, 0);
    puck->calculateLocalInertia(mass, puckInertia);

    //Here we construct the puck with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo puckRigidBodyCI(mass, puckMotionState, puck, puckInertia);
    rigidBodyPuck = new btRigidBody(puckRigidBodyCI);
    rigidBodyPuck->setActivationState(DISABLE_DEACTIVATION);
    rigidBodyPuck->setFriction(0.0);
    rigidBodyPuck->setRestitution(0.9f);
    rigidBodyPuck->setLinearFactor(btVector3(1,0,1));
    rigidBodyPuck->setAngularFactor(btVector3(0,1,0));

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodyPuck, PUCK, puckBouncesOff);
/*-----------------------------------------------------------------------------*/



/*----------------------this is the barrier--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the barrier
    btDefaultMotionState* barrierMotionState = NULL;
    barrierMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -.5)));

    //Here we construct the barrier with no mass and no inertia
    btRigidBody::btRigidBodyConstructionInfo barrierRigidBodyCI(0, barrierMotionState, middleBarrier, btVector3(0, 0, 0));

    btRigidBody* barrierRigidBody = new btRigidBody(barrierRigidBodyCI);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(barrierRigidBody, barrier, barrierDeflects);
/*-----------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////

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

    // if initialized, begin glut main loop
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
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
        
    dynamicsWorld->removeRigidBody(rigidBodyCylinder);
    delete rigidBodyCylinder->getMotionState();
    delete rigidBodyCylinder;
        
    dynamicsWorld->removeRigidBody(wallOneRigidBody);
    delete wallOneRigidBody->getMotionState();
    delete wallOneRigidBody;

    dynamicsWorld->removeRigidBody(wallTwoRigidBody);
    delete wallTwoRigidBody->getMotionState();
    delete wallTwoRigidBody;
      
    dynamicsWorld->removeRigidBody(wallThreeRigidBody);
    delete wallThreeRigidBody->getMotionState();
    delete wallThreeRigidBody;
        
    dynamicsWorld->removeRigidBody(wallFourRigidBody);
    delete wallFourRigidBody->getMotionState();
    delete wallFourRigidBody;

    dynamicsWorld->removeRigidBody(rigidBodyPuck);
    delete rigidBodyPuck->getMotionState();
    delete rigidBodyPuck;

    dynamicsWorld->removeRigidBody(barrierRigidBody);
    delete barrierRigidBody->getMotionState();
    delete barrierRigidBody;

    delete ground;
    delete wallOne;
    delete wallTwo;
    delete wallThree;
    delete wallFour;
    delete paddlePlayer1;
    delete paddlePlayer2;
    delete dynamicsWorld;
    delete puck;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
    delete middleBarrier;

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

  glUseProgram(0);

  if( !started )
  {
    // place text 
    if( !timeflag )
    {
      sPrint(-0.95,0.9, (char*)"Spacebar to Start Simulation", 18);
      sPrint(-0.95,0.8, (char*)"Esc to Quit", 18);
    }
    else
    {
      menuflag = true;
      started = true;
      timeflag = true;
    }

  }

    std::string scoreText1 = "Player 1: " + std::to_string(t1score);
    std::string scoreText2 = "Player 2: " + std::to_string(t2score);

    // if it is time for a new game
    if( newGame )
    {
      // pause game
      timeflag = false;

      // check for winner
      if( t1score == t2score )
      {
        // print results
        std::string winnerStr = "Game restarted. Press space to play.";
        sPrint(-0.9, 0.9, winnerStr.c_str(), 18);
      }
      else if( t1score > t2score )
      {
        // print results
        std::string winnerStr = "Player 2 wins! Press space to play again.";
        sPrint(-0.9, 0.9, winnerStr.c_str(), 18);
      }
      else
      {
        // print results
        std::string winnerStr = "Player 2 wins! Press space to play again.";
        sPrint(-0.9, 0.9, winnerStr.c_str(), 18);
      }
    }
    
    if (menuflag && !newGame && started)
    {
      if( mouseOn )
      {
        sPrint(-0.95,0.9,(char*)"Use Mouse to Move Player 1 Paddle", 12);        
      }
      else
      {
        sPrint(-0.95,0.9,(char*)"WASD to Move Player 1 Paddle", 12);
      }
      if(aiCanMove)
      {
        sPrint(-0.95,0.8,(char*)"AI Moves Player 2 Paddle", 12);
      }
      else
      {
        sPrint(-0.95,0.8,(char*)"Arrow Keys to Move Player 2 Paddle", 12);
      }
      sPrint(-0.95,0.7,(char*)"K to Pan to Player 1 POV (Default)", 12);
      sPrint(-0.95,0.6,(char*)"I to Pan to Player 2 POV", 12);
      sPrint(-0.95,0.5,(char*)"J to Pan to Left Side of Board", 12);
      sPrint(-0.95,0.4,(char*)"L to Pan to Right Side of Board", 12);
      sPrint(-0.95,0.3,(char*)"Spacebar to Pause/Resume", 12);
      if( mouseOn )
      {
        sPrint(-0.95,0.2,(char*)"G to Use WASD for Player 1 Controls", 12);
      }
      else
      {
        sPrint(-0.95,0.2,(char*)"G to Use Mouse for Player 1 Controls", 12);        
      }
      sPrint(-0.95,0.1,(char*)"Right Click for More Options", 12);      
      sPrint(-0.95,0.0,(char*)"H to Hide Menu", 12);
      sPrint(-0.95,-0.1,(char*)"Esc to Quit", 12);
    }

    // display scores
    sPrint(-0.85,-0.9,scoreText1.c_str(), 18);
    sPrint(0.6,-0.9,scoreText2.c_str(), 18);

    // enable the shader program
    glUseProgram(program);

    // loop through each planet
    for( int index = 0; index < numImages; index++ )
    {
      // premultiply the matrix for this example
      images[index].mvp = projection * view * images[index].model;

      // upload the matrix to the shader
      glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, &(images[index].mvp[0][0])); 

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

      glDrawArrays(GL_TRIANGLES, 0, images[index].geometrySize);//mode, starting index, count
    }

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texture);

  //swap the buffers
  glutSwapBuffers();

}


// actions for left mouse click
void mouse(int button, int state, int x_pos, int y_pos)
{
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
}

//Move mouse
void moveMouse(int x, int y)
{

    //update x and y positions
    mouseXAxis = x;
    mouseYAxis = y;
    mouseCanMove = true;

}

// called on idle to update display
void update()
{
  if( timer > 0 )
  {
    timer = timer - 1;
    float dt = getDT();
    rigidBodyPuck->setWorldTransform(puckStart);
    rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));
    dynamicsWorld->stepSimulation(dt, 10);
  }
  else if ( timeflag )
  {
    // update object
    float dt = getDT();
    float force = 10.0;
    btTransform trans;
    float rotationAngle = 0.00;
    float rotationSpeed = 0.03;

    btScalar m[16];
    btScalar m2[16];
    btScalar m3[16];
 
    float forceXDir,forceZDir;
    if( mouseOn )
    {
      // check if the mouse can move 
      if(mouseCanMove)
      {
        if (player1POV)
        {
        if(mouseXAxis < (w/2.1))
            {
                forceXDir = force;
            }
        else if (mouseXAxis > (1.6*w/3))
            {
                forceXDir = -force;
            }
        else
            {
                forceXDir = 0;
            }
        if(mouseYAxis < (h/1.8))
            {
                forceZDir = force;
            }
        else if (mouseYAxis > (2*h/3))
            {
                forceZDir = -force;
            }
        else
            {
                forceZDir = 0;
            }
        rigidBodySphere->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
        mouseCanMove = false;
        }

        if (player2POV)
        {
          if(mouseXAxis < (w/2))
          {
            forceXDir = -force;
          }
          else if (mouseXAxis > (2*w/3))
          {
              forceXDir = force;
          }
          else
          {
            forceXDir = 0;
          }
          if(mouseYAxis < (h/2))
          {
            forceZDir = -force;
          }
          else if (mouseYAxis > (2*h/3))
          {
            forceZDir = force;
          }
          else
          {
            forceZDir = 0;
          }
          rigidBodySphere->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
          mouseCanMove = false;
        }
       

        if (leftSidePOV)
        {
          if(mouseXAxis < (w/2))
          {
              forceZDir = force;
          }
          else if (mouseXAxis > (2*w/3))
          {
            forceZDir = -force;
          }
          else
          {
            forceZDir = 0;
          }
          if(mouseYAxis < (h/2))
          {
              forceXDir = -force;
          }
          else if (mouseYAxis > (2*h/3))
          {
            forceXDir = force;
          }

          else
          {
              forceXDir = 0;
          }
          rigidBodySphere->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
          mouseCanMove = false;
        }


        if (rightSidePOV)
        {
            if(mouseXAxis < (w/2))
            {
                forceZDir = -force;
            }
            else if (mouseXAxis > (2*w/3))
            {
                forceZDir = force;
            }
            else
            {
                forceZDir = 0;
             }

            if(mouseYAxis < (h/2))
            {
                forceXDir = force;
            }
            else if (mouseYAxis > (2*h/3))
            {
                forceXDir = -force;
            }

            else
            {
                forceXDir = 0;
            }
            rigidBodySphere->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
            mouseCanMove = false;
        }
      }
    }

      // add the forces to the paddlePlayer1 for movement
      if(forward)
      {
          rigidBodySphere->applyCentralImpulse(btVector3(0.0,0.0,force));
          //forward = false;
      }
      if(backward)
      {
          rigidBodySphere->applyCentralImpulse(btVector3(0.0,0.0,-force));
          //backward = false;
      }
      if(goLeft)
      {
          rigidBodySphere->applyCentralImpulse(btVector3(force,0.0,0.0));
          //goLeft = false;
      }
      if(goRight)
      {
          rigidBodySphere->applyCentralImpulse(btVector3(-force,0.0,0.0));
          //goRight = false;
      }

          //set the paddlePlayer1 to it's respective model
      rigidBodySphere->getMotionState()->getWorldTransform(trans);
      trans.getOpenGLMatrix(m);
      images[1].model = glm::make_mat4(m);


      if(cylforward)
      {
          rigidBodyCylinder->applyCentralImpulse(btVector3(0.0,0.0,force));
          //cylforward = false;
      }
      if(cylbackward)
      {
          rigidBodyCylinder->applyCentralImpulse(btVector3(0.0,0.0,-force));
          //cylbackward = false;
      }
      if(cylgoLeft)
      {
          rigidBodyCylinder->applyCentralImpulse(btVector3(force,0.0,0.0));
          //cylgoLeft = false;
      }
      if(cylgoRight)
      {
          rigidBodyCylinder->applyCentralImpulse(btVector3(-force,0.0,0.0));
          //cylgoRight = false;
      }

      //set the paddleplayer2 to it's respective model
      rigidBodyCylinder->getMotionState()->getWorldTransform(trans);
      trans.getOpenGLMatrix(m2);
      images[2].model = glm::make_mat4(m2);
      glm::vec4 paddlePos = images[2].model * glm::vec4(1.0f);

      //set the puck to it's respective model
        rigidBodyPuck->getMotionState()->getWorldTransform(trans);
        trans.getOpenGLMatrix(m3);
        images[3].model = glm::make_mat4(m3);
        glm::vec4 puckPos = images[3].model * glm::vec4(1.0f);


    ///// artificial intelligence motion
    if(aiCanMove)
    {
      if (player1POV)
        {
        if( paddlePos.x < puckPos.x )
            {
            if (level1)
                {
                forceXDir = force*1;
                }

            if (level2)
                {
                forceXDir = force*2;
                }

            if (level3)
                {
                forceXDir = force*4;
                }
            }
        else if (paddlePos.x > puckPos.x)
            {
            if (level1)
                { 
                forceXDir = -force*1;
                }

            if (level2)
                { 
                forceXDir = -force*2;
                }

            if (level3)
                { 
                forceXDir = -force*4;
                }
            }
        else
            {
                forceXDir = 0;
            }
        if(paddlePos.z < puckPos.z)
            {
                forceZDir = force;
            }
        else if (paddlePos.z > puckPos.z)
            {
                forceZDir = -force;
            }
        else
            {
                forceZDir = 0;
            }
        rigidBodyCylinder->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
        }    


        if (player2POV)
        {
        if( paddlePos.x < puckPos.x )
            {
            if (level1)
                {
                forceXDir = -force*2;
                }

            if (level2)
                {
                forceXDir = -force*3;
                }

            if (level3)
                {
                forceXDir = -force*5;
                }
            }
        else if (paddlePos.x > puckPos.x)
            {
            if (level1)
                {
                forceXDir = force*2;
                }

            if (level2)
                {
                forceXDir = force*3;
                }

            if (level3)
                {
                forceXDir = force*5;
                }
            }
        else
            {
                forceXDir = 0;
            }
        if(paddlePos.z < puckPos.z)
            {
                forceZDir = -force;
            }
        else if (paddlePos.z > puckPos.z)
            {
                forceZDir = force;
            }
        else
            {
                forceZDir = 0;
            }
        rigidBodyCylinder->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
        }  


        if (leftSidePOV)
        {
        if( paddlePos.x < puckPos.x )
            {
            if (level1)
                {
                forceXDir = force*2;
                }

            if (level2)
                {
                forceXDir = force*3;
                }

            if (level3)
                {
                forceXDir = force*5;
                }
            }
        else if (paddlePos.x > puckPos.x)
            {
            if (level1)
                {
                forceXDir = -force*2;
                }

            if (level2)
                {
                forceXDir = -force*3;
                }

            if (level3)
                {
                forceXDir = -force*5;
                }
            }
        else
            {
                forceXDir = 0;
            }
        if(paddlePos.z < puckPos.z)
            {
                forceZDir = -force;
            }
        else if (paddlePos.z > puckPos.z)
            {
                forceZDir = force;
            }
        else
            {
                forceZDir = 0;
            }
        rigidBodyCylinder->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
        } 



        if (rightSidePOV)
        {
        if( paddlePos.x < puckPos.x )
            {
            if (level1)
                {
                forceXDir = -force*2;
                }

            if (level2)
                {
                forceXDir = -force*3;
                }

            if (level3)
                {
                forceXDir = -force*5;
                }
            }
        else if (paddlePos.x > puckPos.x)
            {
            if (level1)
                {
                forceXDir = force*2;
                }

            if (level2)
                {
                forceXDir = force*3;
                }

            if (level3)
                {
                forceXDir = force*5;
                }
            }
        else
            {
                forceXDir = 0;
            }
        if(paddlePos.z < puckPos.z)
            {
                forceZDir = force;
            }
        else if (paddlePos.z > puckPos.z)
            {
                forceZDir = -force;
            }
        else
            {
                forceZDir = 0;
            }
        rigidBodyCylinder->applyCentralImpulse(btVector3(forceXDir,0.0,forceZDir));
        } 
    }

        if (puckPos.z < -9.7 &&  puckPos.x < 1.5 && puckPos.x > -1.5 )
        {
            rigidBodyPuck->setWorldTransform(puckStart);
            rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));
              t1score = t1score + 1;                                            
              if (t1score == 5)
              {
                newGame = true;
              }
              else
              {
                timer = 20;
              }
        }
        if (puckPos.z > 9.7 &&  puckPos.x < 1.5 && puckPos.x > -1.5)
        {
            rigidBodyPuck->setWorldTransform(puckStart);
            rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));
            t2score = t2score + 1;
            if (t2score == 5)
            {
              newGame = true;
            }    
            else
            {      
              timer = 20;
            }
        }

      // step simluation
      dynamicsWorld->stepSimulation(dt, 10);

      // background rotation
      rotationAngle = dt * rotationSpeed;
      images[numImages-1].model = glm::rotate( images[numImages-1].model, rotationAngle, glm::vec3(0.0, 1.0, 0.0));
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

void keyboardUP(unsigned char key, int x_pos, int y_pos )
{
  if (mouseCanMove)
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
}

// called on keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos )
{
  // Handle keyboard input - end program
    if((key == 27)||(key == 'q')||(key == 'Q'))
    {
      glutLeaveMainLoop();
    }
    // for score cheat - debug purposes
    if(key == 'O')
    {
      t1score++;                                              
      if (t1score == 5)
      {
        newGame = true;
        rigidBodyPuck->setWorldTransform(puckStart);
        rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));
      }
    }
    if(key == 'P')
    {
      t2score++;                                              
      if (t2score == 5)
      {
        newGame = true;
        rigidBodyPuck->setWorldTransform(puckStart);
        rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));
      }
    }
    
 

    if (mouseCanMove)
        {

        if((key == 'w')||(key == 'W'))
        {
        forward = true;
        }
        if((key == 'a')||(key == 'A'))
        {
        goLeft = true;
        }
        if((key == 's')||(key == 'S'))
        {
        backward = true;
        }
        if((key == 'd')||(key == 'D'))
        {
        goRight = true;
        }

        }


    if((key == 'i')||(key == 'I'))
    {
      dest[0] = 0.0;
      dest[1] = 18.0;
      dest[2] = 18.0;
      updateViewFlag = true;        
      pan();
    }
    if((key == 'j')||(key == 'J'))
    {
      dest[0] = 18.0;
      dest[1] = 18.0;
      dest[2] = 0.0; 
      updateViewFlag = true;        
      pan();
    }
    if((key == 'k')||(key == 'K'))
    {
      dest[0] = 0.0;
      dest[1] = 18.0;
      dest[2] = -18.0; 
      updateViewFlag = true;        
      pan();
    }
    if((key == 'l')||(key == 'L'))
    {
      dest[0] = -18.0;
      dest[1] = 18.0;
      dest[2] = 0.0; 
      updateViewFlag = true;        
      pan(); 
    }
    if((key == ' '))
    {
      if( newGame )
      {
        newGame = false;
        rigidBodyPuck->setWorldTransform(puckStart);
        rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));  
        t1score = 0;
        t2score = 0;
        timer = 15;
      }
      timeflag = !timeflag;
    }
    if((key == 'g')||(key == 'G'))
    {
      mouseOn = !mouseOn;
      mouseCanMove = !mouseCanMove;
    }    
    if((key == 'h')||(key == 'H'))
    {
      menuflag = !menuflag;
    }
    glutPostRedisplay();
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


     // loop through each object
      for( index = 0; index < numImages; index++ )
      {
        // Create a Vertex Buffer object to store this vertex info on the GPU
        glGenBuffers(1, &(images[index].vbo_geometry));
        glBindBuffer(GL_ARRAY_BUFFER, images[index].vbo_geometry);
        glBufferData(GL_ARRAY_BUFFER, meshes[index].geometry.size()*sizeof(Vertex), &(meshes[index].geometry[0]), GL_STATIC_DRAW);

        // Create Texture object
        glGenTextures(1, &(images[index].texture));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, images[index].texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, images[index].imageCols, images[index].imageRows, 0, GL_RGBA, GL_UNSIGNED_BYTE, images[index].m_blob.data());
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

    loc_texture = glGetAttribLocation(program, "v_color");
      if(loc_texture == -1)
      {
        std::cerr << "[F] COLOR NOT FOUND" << std::endl;
        return false;
      }      

    // set view variables
    source[0] = 0.0;
    source[1] = 18.0;
    source[2] = -18.0;  
    dest[0] = 0.0;
    dest[1] = 18.0;
    dest[2] = -18.0;   

    player1POV = true;   

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


    rigidBodyPuck->getMotionState()->getWorldTransform(puckStart);
    //images[0].model = glm::scale(images[0].model, glm::vec3(2.3, 2.3, 2.3));
    images[numImages-1].model = glm::scale(images[numImages-1].model, glm::vec3(25, 25, 25));

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
  bool geometryLoadedCorrectly;
  bool imageLoadedCorrectly;
  int index;

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
      geometryLoadedCorrectly = meshes[index].loadMesh( objFilepath.c_str());

        // return false if not loaded
        if( !geometryLoadedCorrectly )
        {
          std::cerr << "[F] GEOMETRY NOT LOADED CORRECTLY" << std::endl;
          return false;
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
    }

}

// adds and removes menus
void manageMenus( bool quitCall )
{
  int index = 0;
  int index2 = 0;
  int mainIndex = 0;

  // upon initialization
  if( !quitCall )
  {
    // create main menu
    index = glutCreateMenu(subMenu);
    glutAddMenuEntry("Player 1 POV (Default)", 1);
    glutAddMenuEntry("Player 2 POV", 2);
    glutAddMenuEntry("Left Side View", 3);
    glutAddMenuEntry("Right Side View", 4);

    index2 = glutCreateMenu(subMenu2);
    glutAddMenuEntry("Start/Stop AI", 1);
    glutAddMenuEntry("AI Difficulty Level 1", 2);
    glutAddMenuEntry("AI Difficulty Level 2", 3);
    glutAddMenuEntry("AI Difficulty Level 3", 4);

    mainIndex = glutCreateMenu(mainMenu);
    glutAddSubMenu("View Options", index);
    glutAddSubMenu("View AI Menu", index2);
    glutAddMenuEntry("Restart Game", 2);
    glutAddMenuEntry("WASD/Mouse Player 1 Controls", 3);    
    glutAddMenuEntry("Pause/Resume Game", 4);
    glutAddMenuEntry("Exit Program", 5);
    glutAttachMenu(GLUT_RIGHT_BUTTON); //Called if there is a mouse click (right)
  }

  // destroy menus before ending program
  else
  {
    // clean up after ourselves
    glutDestroyMenu(index2);
    glutDestroyMenu(mainIndex);
    glutDestroyMenu(index);
  }

  // update display
  glutPostRedisplay();
}


void subMenu2 (int num)
{
  switch(num)
    {
    case 1: 
      aiCanMove = !aiCanMove;
      break;
    case 2:
      level1 = true;
      level2 = false;
      level3 = false;
      break;
    case 3: 
      level1 = false;
      level2 = true;
      level3 = false;
      break;
    case 4: 
      level1 = false;
      level2 = false;
      level3 = true;
      break;      
    }

}






//the first menu that appears
void subMenu(int num)
{
  switch(num)
    {
    case 1: // Player 1 POV 
      dest[0] = 0.0;
      dest[1] = 18.0;
      dest[2] = -18.0;
      source[0] = 0.0;
      source[1] = 18.0;
      source[2] = -18.0;
      updateViewFlag = true; 
      player1POV = true;
      player2POV = false;  
      leftSidePOV = false; 
      rightSidePOV = false;    
      pan();
      break;
    case 2: // Player 2 POV

      dest[0] = 0.0;
      dest[1] = 18.0;
      dest[2] = 18.0;
      source[0] = 0.0;
      source[1] = 18.0;
      source[2] = 18.0;
      updateViewFlag = true; 
      player1POV = false;
      player2POV = true;  
      leftSidePOV = false; 
      rightSidePOV = false;        
      pan();
      break;
    case 3: // Left side POV
  
      dest[0] = 18.0;
      dest[1] = 18.0;
      dest[2] = 0.0;
      source[0] = 18.0;
      source[1] = 18.0;
      source[2] = 0.0;
      updateViewFlag = true;  
      player1POV = false;
      player2POV = false;  
      leftSidePOV = true; 
      rightSidePOV = false;        
      pan();
      break;
    case 4: // Right side POV
      
      dest[0] = -18.0;
      dest[1] = 18.0;
      dest[2] = 0.0;
      source[0] = -18.0;
      source[1] = 18.0;
      source[2] = 0.0;
      updateViewFlag = true;
      player1POV = false;
      player2POV = false;  
      leftSidePOV = false; 
      rightSidePOV = true;          
      pan();
      break;      
    }
  glutPostRedisplay();
}

//the second sub-menu
void mainMenu(int num)
{
  switch (num)
    {
    case 1: // view options
      subMenu(num);
      break;
    case 2: // restart game
      newGame = true;
      t1score = 0;
      t2score = 0;
      rigidBodyPuck->setWorldTransform(puckStart);
      rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));      
      break;
    case 3: // swap controls for player 1
      mouseOn = !mouseOn;
      break;
    case 4: // pause
      if( newGame )
      {
        newGame = false;
        rigidBodyPuck->setWorldTransform(puckStart);
        rigidBodyPuck->setLinearVelocity(btVector3(0.0,0.0,0.0));         
        t1score = 0;
        t2score = 0;
      }
      timeflag = !timeflag;
      break;
    case 5: // quit program
      glutLeaveMainLoop();
      //exit(0);
      break;
    }
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

    // check if paused
    if( !timeflag )
    {
      return 0.0;
    }

    return ret;
}

void arrowKeysUp(int button, int x_pos, int y_pos)
{
  if (!aiCanMove)
    {
    if (button == GLUT_KEY_LEFT)
        {
        cylgoLeft = false;
        }

    if (button == GLUT_KEY_RIGHT)
        {
        cylgoRight = false;
        }

    if (button == GLUT_KEY_UP)
        {
        cylforward = false;
        }

    if (button == GLUT_KEY_DOWN)
        {
        cylbackward = false;
        }
    }
}

void arrowKeys(int button, int x_pos, int y_pos)
{
  if (!aiCanMove)
    {
    if (button == GLUT_KEY_LEFT)
        {
        cylgoLeft = true;
        }
    
    if (button == GLUT_KEY_RIGHT)
        {
        cylgoRight = true;
        }

    if (button == GLUT_KEY_UP)
        {
        cylforward = true;
        }

    if (button == GLUT_KEY_DOWN)
        {
        cylbackward = true;
        }
    }
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



        if ( (dest[0] == 0.0) && (dest[1] == 18.0) && (dest[2] == -18.0) )
            {
            player1POV = true;
            player2POV = false;  
            leftSidePOV = false; 
            rightSidePOV = false; 
            }

        if ( (dest[0] == 0.0) && (dest[1] == 18.0) && (dest[2] == 18.0) )
            {
            player1POV = false;
            player2POV = true;  
            leftSidePOV = false; 
            rightSidePOV = false; 
            }

        if ( (dest[0] == 18.0) && (dest[1] == 18.0) && (dest[2] == 0.0) )
            {
            player1POV = false;
            player2POV = false;  
            leftSidePOV = true; 
            rightSidePOV = false; 
            }

        if ( (dest[0] == -18.0) && (dest[1] == 18.0) && (dest[2] == 0.0) )
            {
            player1POV = false;
            player2POV = false;  
            leftSidePOV = false; 
            rightSidePOV = true; 
            }


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
