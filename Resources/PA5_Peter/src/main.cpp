#include <GL/glew.h> // glew must be included before the main gl libs
//#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <GL/freeglut.h>
#include <iostream>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include <assimp/Importer.hpp> //includes the importer, which is used to read our obj file
#include <assimp/scene.h> //includes the aiScene object
#include <assimp/postprocess.h> //includes the postprocessing variables for the importer
#include <assimp/color4.h> //includes the aiColor4 object, which is used to handle the colors from the mesh objects

#include <fstream>
#include "shaderClass.h"
#include "shaderClass.cpp"
#include <cstring>
#include <cstdio>
#include <vector>



//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry
int cube_orbit = -1;
int cube_rotation = -1;
bool object_rotation = false;// start the object as NOT rotating


// Even More Evil global constants
const char* vsFile = "../bin/shader.vs";
const char* fsFile = "../bin/shader.fs";

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void idle();
std::vector<Vertex> geometry;
bool loadOBJ(char * obj, std::vector<Vertex>& geometry);



//--Resource management
bool initialize(char* fileName);
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;


//--Main
int main(int argc, char **argv)
{
    // copy the file name given by the user
    // into an array
    char* fileName = new char[100];
    if (argc > 1)//if correct name that exists
        {
        strcpy (fileName, argv[1]); //copy file name into array
        }
    else
        {
        std::cerr << "[F] UNABLE TO LOCATE THE CORRECT FILE!"<<std::endl;
        return -1;
        }


    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

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


    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(fileName);
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
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

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

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

    glDrawArrays(GL_TRIANGLES, 0, geometry.size());//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float rotationAngle = 0.0;
    static float orbitAngle = 0.0;
    float dt = getDT();// if you have anything moving, use dt.

        orbitAngle += dt * M_PI/2; //move through 90 degrees a second

        rotationAngle += dt * M_PI/2; //move through 90 degrees a second


    // added for the rotation of cube around itself
    model = glm::rotate( glm::mat4(1.0f), rotationAngle, glm::vec3(0.0, 1.0, 0.0));

    // Update the state of the scene
    glutPostRedisplay();//call the display callback
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

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if(key == 27)//ESC
    {
        glutLeaveMainLoop();
    }
}




void idle()
{
glutPostRedisplay();
}

bool initialize(char* fileName)
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    bool geometryLoadedCorrectly = loadOBJ (fileName, geometry); 
    if (!geometryLoadedCorrectly)
        {
        std::cerr << "[F] The Geometry Was Not Loaded Correctly!"<<std::endl;
        return false;
        }  

    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, geometry.size()*sizeof(Vertex), &geometry[0], GL_STATIC_DRAW);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // check if shaders are created
        if(!vertex_shader)
    {
        std::cerr << "Error creating shader vertex!" << std::endl;
        return false;
    }

        if(!fragment_shader)
    {
        std::cerr << "Error creating shader fragment!" << std::endl;
        return false;
    }



    //Shader Sources
    // Put these into files and write a loader in the future
    // Note the added uniform!
    shaderLoaderClass loadVertex;
    shaderLoaderClass loadFragment;

    std::string vsString = loadVertex.loadShader (vsFile);
    std::string fsString = loadFragment.loadShader (fsFile);  

    const char *vs = vsString.c_str ();
    const char *fs = fsString.c_str (); 


    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
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


bool loadOBJ(char * obj, std::vector<Vertex>& geometry)
{
    // Initialize variables
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(obj, aiProcess_Triangulate 
                                            | aiProcess_GenSmoothNormals 
                                            | aiProcess_FlipUVs);

    if(!pScene)
    {
     std::cout<<"Error parsing "<<obj/*<<": "<<Importer.GetErrorString*/<<std::endl;
     return false;
    }

    aiMesh* mesh = pScene->mMeshes[0];

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
        const aiFace& face = mesh->mFaces[i];

        aiVector3D tempPos1 = mesh->mVertices[face.mIndices[0]];
        aiVector3D tempPos2 = mesh->mVertices[face.mIndices[1]];
        aiVector3D tempPos3 = mesh->mVertices[face.mIndices[2]];

        Vertex tempVertex;
    
        // first point of triange
        tempVertex.position[0] = tempPos1.x;
        tempVertex.position[1] = tempPos1.y;
        tempVertex.position[2] = tempPos1.z;

        if (tempVertex.position[0] < 0)
            {
            tempVertex.color[0] = 1.0f;
            tempVertex.color[1] = 0.0f;
            tempVertex.color[2] = 0.0f;
            }
        else
            {
            tempVertex.color[0] = 0.0f;
            tempVertex.color[1] = 1.0f;
            tempVertex.color[2] = 0.0f;
            }

        geometry.push_back(tempVertex);

        // second point of triange
        tempVertex.position[0] = tempPos2.x;
        tempVertex.position[1] = tempPos2.y;
        tempVertex.position[2] = tempPos2.z;

        if (tempVertex.position[0] < 0)
            {
             tempVertex.color[0] = 1.0f;
            tempVertex.color[1] = 0.0f;
            tempVertex.color[2] = 0.0f;
            }
        else
            {
            tempVertex.color[0] = 0.0f;
            tempVertex.color[1] = 1.0f;
            tempVertex.color[2] = 0.0f;
            }

        geometry.push_back(tempVertex);

        // third point on triange
        tempVertex.position[0] = tempPos3.x;
        tempVertex.position[1] = tempPos3.y;
        tempVertex.position[2] = tempPos3.z;

        if (tempVertex.position[0] < 0)
            {
            tempVertex.color[0] = 1.0f;
            tempVertex.color[1] = 0.0f;
            tempVertex.color[2] = 0.0f;
            }
        else
            {
            tempVertex.color[0] = 0.0f;
            tempVertex.color[1] = 1.0f;
            tempVertex.color[2] = 0.0f;
            }

        geometry.push_back(tempVertex);
        }

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
    // if object not rotating set time to t1

        t1 = std::chrono::high_resolution_clock::now();

    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}































