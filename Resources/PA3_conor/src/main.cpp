#define GLM_FORCE_RADIANS
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <chrono>
#include "ShaderLoader.cpp"
#include <string.h>
#include <vector>
#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

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
GLuint TextProgram;
GLuint vbo_geometry;// VBO handle for our geometry

//bool camera flag
bool sunview=true;

bool viewflag = true;

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
glm::mat4 Moon;
glm::mat4 Sun;
glm::mat4 temp;

// Flags
int Spin = 1;
int Rotation = 1;
float Speed = 0;
char* PlanetStatus = (char*)"Planet: Counterclockwise";
char* MoonStatus = (char*)"Moon: Clockwise";

int MoonSpin = -1;
float MoonSpeed = 0;
int MoonRotation = -1;


int Vertices;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void ArrowKeys(int button, int x_pos, int y_pos);
void keyboard(unsigned char key, int x_pos, int y_pos);
void Mouse(int button, int state, int x_pos, int y_pos);

//--Resource management
bool initialize(char* arg);
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

void Menu(int id);
void idle();
void Sprint( float x, float y, char *st);

Vertex* LoadObject(char* FilePath);


//--Main
int main(int argc, char **argv)
{
    TextProgram = 0;

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

    glutCreateMenu(Menu);
    glutAddMenuEntry("Quit", 1);
    glutAddMenuEntry("[LEFT MOUSE] to reverse planet orbit", 2);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);

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
    glutMouseFunc(Mouse);// Called if there is mouse input
    glutSpecialFunc(ArrowKeys);

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(argv[1]);
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

    glUseProgram(TextProgram);
    char* Text = new char[100];
    char* Text2 = new char[100];
    Text = (char*)"A=Reverse Planet Rotation    S=Reverse Moon Rotation";
    Text2 = (char*)"D=Reverse Planet Orbit       F=Reverse Moon Orbit"; 
    Sprint(-1.0,0.9,Text);
    Sprint(-1.0,0.75,Text2);
    Sprint(-0.8,-0.8, PlanetStatus);
    Sprint(0.2,-0.8, MoonStatus);

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

    glDrawArrays(GL_TRIANGLES, 0, Vertices);//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
////////////////////

    mvp = projection * view * Moon;

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

    glDrawArrays(GL_TRIANGLES, 0, Vertices);//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);


/////////////////////////////


    mvp = projection * view * Sun;

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

    glDrawArrays(GL_TRIANGLES, 0, Vertices);//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float angle = 0.0;
    static float rotate = 0.0;
    static float MoonAngle = 0.0;
    static float MoonRotate = 0.0;

    static float x = 26.5 * sin(angle);
    static float y = 0.0;
    static float z = 26.5 * cos(angle);

    static float x2 = 0.0;
    static float y2 = 8.0;
    static float z2 = -35.0;

    static float vx = 16.5 * sin(angle);
    static float vy = 0.0;
    static float vz = 16.5 * cos(angle);

    static float vx2 = 0.0;
    static float vy2 = 0.0;
    static float vz2 = 0.0;

    float dt = getDT();// if you have anything moving, use dt.

    angle += (dt * M_PI/5)*(Spin); //move through 90 degrees a second
    rotate += dt * M_PI * (Rotation);
    MoonAngle += (dt * M_PI/2)*(MoonSpin);
    MoonRotate += dt * M_PI * (MoonRotation);

     x = 26.5 * sin(angle);
     y = 0.0;
     z = 26.5 * cos(angle);

      vx = 16.5 * sin(angle);
      vy = 0.0;
      vz = 16.5 * cos(angle);

    static int counter = 0;



    Sun = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));
    model = glm::translate( glm::mat4(1.0f), glm::vec3(16.5 * sin(angle), 0.0, 16.5 * cos(angle)));
    Moon = glm::translate( model, glm::vec3(5.5 * sin(MoonAngle), 0.0, 5.5 * cos(MoonAngle)));
    model = glm::rotate( model, rotate, glm::vec3(0.0, 1.0, 0.0) );
    Moon = glm::rotate( Moon, MoonRotate, glm::vec3(0.0, 1.0, 0.2) );
    Sun = glm::rotate( Sun, MoonRotate/2, glm::vec3(1.0, 1.0, 0.0) );


    Moon = glm::scale(Moon, glm::vec3(0.75, 0.75, 0.75));
    model = glm::scale(model, glm::vec3(1.25, 1.25, 1.25));
    Sun = glm::scale(Sun, glm::vec3(5.0, 5.0, 5.0));
/*
      temp = glm::lookAt( glm::vec3(x, y, z), //Eye Position
                          glm::vec3(0.0, 0.0, 0.0), //Focus point
                          glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
*/
    if (sunview)
    {
/*
      if (counter == 0)
      {
          x = 0.0;
          y = 8.0;
          z = -35.0;
          vx = 0.0;
          vy = 0.0;
          vz = 0.0;
      }

      if (!viewflag)
      {
        if (x2 < x)
          x2 += 0.14;
        else if (x2 > x)
          x2 -= 0.14;

        else if (y2 < y)
          y2 += 0.14;
        else if (y2 > y)
          y2 -= 0.14;

        if (z2 < z)
          z2 += 0.14;
        else if (z2 > z)
          z2 -= 0.14;

        if (vx2 < vx)
          vx2 += 0.14;
        else if (vx2 > vx)
          vx2 -= 0.14;

        if (vy2 < vy)
          vy2 += 0.14;
        else if (vy2 > vy)
          vy2 -= 0.14;

        if (vz2 < vz)
          vz2 += 0.14;
        else if (vz2 > vz)
          vz2 -= 0.14;
        
      if (counter >= 500)
      {
        counter = 0;
        viewflag = true;
      }
        counter ++;

      view = glm::lookAt( glm::vec3(x2, y2, z2), //Eye Position
                          glm::vec3(vx2, vy2, vz2), //Focus point
                          glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
      }

      else 
*/
        view = glm::lookAt( glm::vec3(0.0, 8.0, -35.0), //Eye Position
                            glm::vec3(0.0, 0.0, 0.0), //Focus point
                            glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
      



      x2 = 0;
      y2 = 8.0;
      z2 = -35.0; 
      vx2 = 0.0;
      vy2 = 0.0;
      vz2 = 0.0;    

    }

    else
    {
      if (counter == 0)
      {
          x = 26.5 * sin(angle);
          y = 0.0;
          z = 26.5 * cos(angle);
          vx = 16.5 * sin(angle);
          vy = 0.0;
          vz = 16.5 * cos(angle);
      }

      if (viewflag)
      {
        if (x2 < x)
          x2 += 0.15;
        if (x2 > x)
          x2 -= 0.15;

        if (y2 < y)
          y2 += 0.15;
        if (y2 > y)
          y2 -= 0.15;

        if (z2 < z)
          z2 += 0.15;
        if (z2 > z)
          z2 -= 0.15;

        if (vx2 < vx)
          vx2 += 0.15;
        if (vx2 > vx)
          vx2 -= 0.15;

        if (vy2 < vy)
          vy2 += 0.15;
        if (vy2 > vy)
          vy2 -= 0.15;

        if (vz2 < vz)
          vz2 += 0.15;
        if (vz2 > vz)
          vz2 -= 0.15;
        
      if (/*counter >= 500*/((x2 - x <= 0.5) && (x2-x >= -0.5))&&
                            ((y2 - y <= 0.5) && (y2-y >= -0.5))&&
                            ((z2 - z <= 0.5) && (z2-z >= -0.5))&&
                            ((vx2 - vx <= 0.5) && (vx2-vx >= -0.5))&&
                            ((vy2 - vy <= 0.5) && (vy2-vy >= -0.5))&&
                            ((vz2 - vz <= 0.5) && (vz2-vz >= -0.5)))
      {
        view = temp;
        counter = 0;
        viewflag = false;
      }
        counter ++;

      view = glm::lookAt( glm::vec3(x2, y2, z2), //Eye Position
                          glm::vec3(vx2, vy2, vz2), //Focus point
                          glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
      }

      else 
              view = glm::lookAt(glm::vec3(26.5 * sin(angle), 0.0, 26.5 * cos(angle)), //Eye Position
                          glm::vec3(16.5 * sin(angle), 0.0, 16.5 * cos(angle)), //Focus point
                          glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
      


    }


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

void ArrowKeys(int button, int x_pos, int y_pos)
{
    if (button == GLUT_KEY_DOWN)
    {
        Rotation = 1;
        PlanetStatus = (char*)"Planet: Counterclockwise";
    }

    if (button == GLUT_KEY_UP)
    {
        Rotation = -1;
        PlanetStatus = (char*)"Planet: Clockwise";
    }

    if (button == GLUT_KEY_LEFT)
        sunview = true;

    if (button == GLUT_KEY_RIGHT)
    {
        sunview = false;
        viewflag = true;
    }
}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if(key == 27)//ESC
    {
        exit(0);
    }

    switch (key)
    {
        case 97: 
            Rotation *= -1;
            if (Rotation < 0)
                PlanetStatus = (char*)"Planet: Clockwise";
            else
                PlanetStatus = (char*)"Planet: Counterclockwise";
            break;

        case 115:
            MoonRotation *= -1;
            if (MoonRotation < 0)
                MoonStatus = (char*)"Moon: Clockwise";
            else
                MoonStatus = (char*)"Moon: Counterclockwise";
            break;

        case 100:
            Spin *= -1;
            break;

        case 102:
            MoonSpin *= -1;
            break;

        default:
            break;
    }
}

void Mouse(int button, int state, int x_pos, int y_pos)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        Spin *= -1;
}

bool initialize(char* arg)
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    Vertex* geometry = LoadObject(arg);

    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, Vertices * sizeof(Vertex), geometry, GL_STATIC_DRAW);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    // Put these into files and write a loader in the future
    // Note the added uniform!
    const char *vs = ShaderLoader::LoadShader("VertexShader.fs");
//std::cout<<vs;
/*
        "attribute vec3 v_position;"
        "attribute vec3 v_color;"
        "varying vec3 color;"
        "uniform mat4 mvpMatrix;"
        "void main(void){"
        "   gl_Position = mvpMatrix * vec4(v_position, 1.0);"
        "   color = v_color;"
        "}";
*/

    const char *fs = ShaderLoader::LoadShader("FragmentShader.fs");
//std::cout<<fs;
/*
        "varying vec3 color;"
        "void main(void){"
        "   gl_FragColor = vec4(color.rgb, 1.0);"
        "}";

*/

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
    view = glm::lookAt( glm::vec3(0.0, 8.0, -35.0), //Eye Position
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

void Menu(int id)
{
    switch(id)
    {
        case 1:
            exit(0);
            break;

        default:
            break;
    }
    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}


// This prints a string to the screen
void Sprint( float x, float y, char *st)
{
    int l,i;

    l=strlen( st ); // see how many characters are in text string.
    glRasterPos2f(x, y); // location to start printing text
    for( i=0; i < l; i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, st[i]); // Print a character on the screen
    }
}


Vertex* LoadObject(char* FilePath)
{
    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    bool HasUV = false, HasNormals = false;

    FILE * file = fopen(FilePath, "r");
    if( file == NULL )
    {
        printf("Impossible to open the file !\n");
        return 0;
    }


    while( 1 ){
     
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
        
        else if ( strcmp( lineHeader, "vt" ) == 0 )
        {
            HasUV = true;
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            temp_uvs.push_back(uv);

        }
        
        else if ( strcmp( lineHeader, "vn" ) == 0 )
        {
            HasNormals = true;
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }

        else if ( strcmp( lineHeader, "f" ) == 0 )
        {         
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

            if (HasNormals && HasUV)
            {
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], 
                                                    &normalIndex[0], &vertexIndex[1], &uvIndex[1], 
                                                    &normalIndex[1], &vertexIndex[2], 
                                                    &uvIndex[2], &normalIndex[2] );
                if (matches != 9)
                {
                    printf("1File can't be read by our simple parser : ( Try exporting with other options\n");
                    return 0;
                }

                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
            }

            else if (HasNormals && !HasUV)
            {
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], 
                                                    &normalIndex[0], &vertexIndex[1], 
                                                    &normalIndex[1], &vertexIndex[2], 
                                                    &normalIndex[2] );
                if (matches != 6)
                {
                    printf("2File can't be read by our simple parser : ( Try exporting with other options\n");
                    return 0;
                }

                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
            }

            else if (!HasNormals && HasUV)
            {
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], 
                                                    &uvIndex[0], &vertexIndex[1], 
                                                    &uvIndex[1], &vertexIndex[2], 
                                                    &uvIndex[2] );
                if (matches != 6)
                {
                    printf("3File can't be read by our simple parser : ( Try exporting with other options\n");
                    return 0;
                }

                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
            }

            else if (!HasNormals && !HasUV)
            {
                int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                if (matches != 3)
                {
                    printf("4File can't be read by our simple parser : ( Try exporting with other options\n");
                    return 0;
                }

                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
            }
        }
    }

    Vertex* Geometry = new Vertex[vertexIndices.size()];
    Vertices = vertexIndices.size();
std::cout<<vertexIndices.size()<<'\n';
    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ )
    {
        unsigned int vertexIndex = vertexIndices[i];

        Geometry[i].position[0] = temp_vertices[ vertexIndex-1 ].x;
        Geometry[i].position[1] = temp_vertices[ vertexIndex-1 ].y;
        Geometry[i].position[2] = temp_vertices[ vertexIndex-1 ].z;

      // set color for each vertex
      if (Geometry[i].position[0] < -0.4)
      {
        Geometry[i].color[0] = 1.0f;
        Geometry[i].color[1] = 0.0f;
        Geometry[i].color[2] = 0.0f;
      }
      else if (Geometry[i].position[0] > -0.4 && Geometry[i].position[0] < 0.4)
      {
        Geometry[i].color[0] = 0.0f;
        Geometry[i].color[1] = 1.0f;
        Geometry[i].color[2] = 0.0f;
      }

      else if (Geometry[i].position[0] > 0.4)
      {
        Geometry[i].color[0] = 0.0f;
        Geometry[i].color[1] = 0.0f;
        Geometry[i].color[2] = 1.0f;
      }

    }
            
    return Geometry;
}




















