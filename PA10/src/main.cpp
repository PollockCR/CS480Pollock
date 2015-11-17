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
  int w = 640, h = 480;

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
  void Menu1(int num);
  void Menu2(int num);
  void mouse(int button, int state, int x_pos, int y_pos);
  void ArrowKeys(int button, int x_pos, int y_pos);

  //--Resource management
  bool initialize( const char* filename);
  void cleanUp();

  //--Time function
  float getDT();

  //Load Image info
  bool loadInfo( const char* infoFilepath, std::vector<Mesh> &meshes, int numOfImages );

  void Sprint( float x, float y, char *st);

  //Bullet 
  btDiscreteDynamicsWorld *dynamicsWorld;
  btRigidBody *rigidBodySphere;
  btRigidBody *rigidBodyCube;
  btRigidBody *rigidBodyCylinder;

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


// MAIN FUNCTION
int main(int argc, char **argv)
{
    bool init = false;

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    // Name and create the Window
    glutCreateWindow("Bullet Project");

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
    glutMouseFunc(mouse);//Called if there is mouse input
    glutSpecialFunc(ArrowKeys);
	int index = glutCreateMenu(Menu1);
	glutAddMenuEntry("Rotate Clockwise", 1);
	glutAddMenuEntry("Rotate Counterclockwise", 2);
	glutAddMenuEntry("Don't Rotate", 3);
	glutCreateMenu(Menu2);
	glutAddSubMenu("Rotation options", index);
	glutAddMenuEntry("Exit Program", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	srand(getDT());

    // add menus
    manageMenus( false );

//////////////////////////////////////////////////////////////////////////
    //create brodphase
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();

    //create collision configuration
    btDefaultCollisionConfiguration* collisionConfiguration = new       btDefaultCollisionConfiguration();

    //create a dispatcher
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

    //create a solver
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    //create the physics world
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

     //set the gravity
     dynamicsWorld->setGravity(btVector3(0, -10, 0));

    //create a game board which the objects will be on
    btCollisionShape* ground = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    btCollisionShape* wallOne = new btStaticPlaneShape(btVector3(-1, 0, 0), 1);
    btCollisionShape* wallTwo = new btStaticPlaneShape(btVector3(1, 0, 0), 1);
    btCollisionShape* wallThree = new btStaticPlaneShape(btVector3(0, 0, 1), 1);
    btCollisionShape* wallFour = new btStaticPlaneShape(btVector3(0, 0, -1), 1);

  //create sphere and set radius to 1
    btCollisionShape* sphere = new btSphereShape(1);

    //create cube and set extents to 0.5 each
    btCollisionShape* cube = new btBoxShape(btVector3(0.5,0.5,0.5));

    //create a cylinder and set radius of each axis to 1.0
    btCollisionShape* cylinder = new btCylinderShape(btVector3(1.0,1.0,1.0));


/*----------------------this is the gameboard--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the ground
    btDefaultMotionState* groundMotionState = NULL;
    groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
    //here we construct the ground using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, ground, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(groundRigidBody);
        
    ////make the first wall
    btDefaultMotionState* wallOneMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(5.6, 0, 0)));
    //here we construct the first wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallOneRigidBodyCI(0, wallOneMotionState, wallOne, btVector3(0, 0, 0));
    btRigidBody* wallOneRigidBody = new btRigidBody(wallOneRigidBodyCI);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallOneRigidBody);


    ////make the second wall
    btDefaultMotionState* wallTwoMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-5.6, 0, 0)));
    //here we construct the second wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallTwoRigidBodyCI(0, wallTwoMotionState, wallTwo, btVector3(0, 0, 0));
    btRigidBody* wallTwoRigidBody = new btRigidBody(wallTwoRigidBodyCI);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallTwoRigidBody);


    ////make the third wall FRONTBACK
    btDefaultMotionState* wallThreeMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -5.6)));
    //here we construct the third wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallThreeRigidBodyCI(0, wallThreeMotionState, wallThree, btVector3(0, 0, 0));
    btRigidBody* wallThreeRigidBody = new btRigidBody(wallThreeRigidBodyCI);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallThreeRigidBody);


    ////make the fouth wall FRONTBACK
    btDefaultMotionState* wallFourMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 5.6)));
    //here we construct the fourth wall using the motion state and shape
    btRigidBody::btRigidBodyConstructionInfo wallFourRigidBodyCI(0, wallFourMotionState, wallFour, btVector3(0, 0, 0));
    btRigidBody* wallFourRigidBody = new btRigidBody(wallFourRigidBodyCI);
        
    //display dynamic body in our world
    dynamicsWorld->addRigidBody(wallFourRigidBody);
/*-----------------------------------------------------------------------------*/


/*----------------------this is the sphere--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the sphere
    btDefaultMotionState* sphereMotionState = NULL;
    sphereMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(2, 1, 0)));

    // the sphere must have a mass
    btScalar mass = 1;

    //we need the inertia of the sphere and we need to calculate it
    btVector3 sphereInertia(0, 0, 0);
    sphere->calculateLocalInertia(mass, sphereInertia);

    //Here we construct the sphere with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI(mass, sphereMotionState, sphere, sphereInertia);
    rigidBodySphere = new btRigidBody(sphereRigidBodyCI);
    rigidBodySphere->setActivationState(DISABLE_DEACTIVATION);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodySphere);
/*-----------------------------------------------------------------------------*/
        
/*----------------------this is the cube--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the cube
    btDefaultMotionState* cubeMotionState = NULL;
    cubeMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 1, 0)));

    //the cube must have a mass
    mass = 0;

    //we need the inertia of the cube and we need to calculate it
    btVector3 cubeInertia(0, 0, 0);
    cube->calculateLocalInertia(mass, cubeInertia);

    //Here we construct the cube with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo cubeRigidBodyCI(mass, cubeMotionState, sphere, sphereInertia);
    rigidBodyCube = new btRigidBody(cubeRigidBodyCI);

    // this is where we make a static object (like the cube) into a kinematic object
    rigidBodyCube->setCollisionFlags(rigidBodyCube->getCollisionFlags() |  btCollisionObject::CF_KINEMATIC_OBJECT);
    rigidBodyCube->setActivationState(DISABLE_DEACTIVATION);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodyCube);
/*-----------------------------------------------------------------------------*/
        
/*----------------------this is the cylinder--------------------------------*/        
  // After we create collision shapes we have to se the default motion state 
    // for the cylinder
    btDefaultMotionState* cylinderMotionState = NULL;
    cylinderMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(1, 20, 1)));

    //the cylinder must have a mass
    mass = 1;

    //we need the inertia of the cylinder and we need to calculate it
    btVector3 cylinderInertia(0, 0, 0);
    cylinder->calculateLocalInertia(mass, cylinderInertia);

    //Here we construct the cylinder with a mass, motion state, and inertia
    btRigidBody::btRigidBodyConstructionInfo cylinderRigidBodyCI(mass, cylinderMotionState, cylinder, sphereInertia);
    rigidBodyCylinder = new btRigidBody(cylinderRigidBodyCI);
    rigidBodyCylinder->setActivationState(DISABLE_DEACTIVATION);

    //display dynamic body in our world
    dynamicsWorld->addRigidBody(rigidBodyCylinder);
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
        
    dynamicsWorld->removeRigidBody(rigidBodyCube);
    delete rigidBodyCube->getMotionState();
    delete rigidBodyCube;
        
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

    delete ground;
    delete wallOne;
    delete wallTwo;
    delete wallThree;
    delete wallFour;
    delete sphere;
    delete cube;
    delete cylinder;
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;

    cleanUp();
    return 0;
}

// FUNCTION IMPLEMENTATION

//the first menu that appears
void Menu1(int num)
    {
	switch(num)
		{
		case 1: 
			rotationDegrees = 45.0;
			break;
		case 2: 
			rotationDegrees =-45.0;
			break;
		case 3: 
			rotationDegrees = 0.0;
			break;
		}
	glutPostRedisplay();
	}

//the second sub-menu
void Menu2(int num)
    {
	switch (num)
		{
		case 1:
			Menu1(num);
			break;
		case 2:
			exit(0);
			break;
		}
	glutPostRedisplay();
	}


// render the scene
void render()
{
  // clear the screen
  glClearColor(0.0, 0.0, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);
    char* Text = new char[100];
    
    Text = (char*) "WASD to move sphere, Arrow keys to move cylinder";
    Sprint(-0.7,0.9,Text);

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

// called on idle to update display
void update()
{
  // update object
    float dt = getDT();
    float force = 10.0;
    
    // add the forces to the sphere for movement
    if(forward)
    {
        rigidBodySphere->applyCentralImpulse(btVector3(0.0,0.0,force));
        forward = false;
    }
    if(backward)
    {
        rigidBodySphere->applyCentralImpulse(btVector3(0.0,0.0,-force));
        backward = false;
    }
    if(goLeft)
    {
        rigidBodySphere->applyCentralImpulse(btVector3(force,0.0,0.0));
        goLeft = false;
    }
    if(goRight)
    {
        rigidBodySphere->applyCentralImpulse(btVector3(-force,0.0,0.0));
        goRight = false;
    }
    if(cylforward)
    {
        rigidBodyCylinder->applyCentralImpulse(btVector3(0.0,0.0,force));
        cylforward = false;
    }
    if(cylbackward)
    {
        rigidBodyCylinder->applyCentralImpulse(btVector3(0.0,0.0,-force));
        cylbackward = false;
    }
    if(cylgoLeft)
    {
        rigidBodyCylinder->applyCentralImpulse(btVector3(force,0.0,0.0));
        cylgoLeft = false;
    }
    if(cylgoRight)
    {
        rigidBodyCylinder->applyCentralImpulse(btVector3(-force,0.0,0.0));
        cylgoRight = false;
    }

    
    dynamicsWorld->stepSimulation(dt, 10);


    btTransform trans;

    btScalar m[16];
    btScalar m1[16];
    btScalar m2[16];

    //set the sphere to it's respective model
    rigidBodySphere->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(m);
    images[1].model = glm::make_mat4(m);

   
    //set the cube to it's respective model
    rigidBodyCube->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(m1);
    images[2].model = glm::make_mat4(m1);
   
    //set the cylinder to it's respective model
    rigidBodyCylinder->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(m2);
    images[3].model = glm::make_mat4(m2);

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

// called on keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos )
{
  // Handle keyboard input - end program
    if((key == 27)||(key == 'q')||(key == 'Q'))
        {
        glutLeaveMainLoop();
        }
    else if((key == 'w')||(key == 'W'))
        {
        forward = true;
        }
    else if((key == 'a')||(key == 'A'))
        {
        goLeft = true;
        }
    else if((key == 's')||(key == 'S'))
        {
        backward = true;
        }
    else if((key == 'd')||(key == 'D'))
        {
        goRight = true;
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

    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 5.0, -10.0), //Eye Position
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
    return ret;
}

void ArrowKeys(int button, int x_pos, int y_pos)
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

// This prints a string to the screen
void Sprint( float x, float y, char *st)
{
    int l,i;

    l=strlen( st ); // see how many characters are in text string.
    glRasterPos2f(x, y); // location to start printing text
    for( i=0; i < l; i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st[i]); // Print a character on the screen
    }
}