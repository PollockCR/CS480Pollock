attribute vec3 v_position;
attribute vec3 vertexPosition_modelspace;
attribute vec2 v_color;
attribute vec3 vertexNormal_modelspace;

varying vec2 color;
varying vec3 Position_worldspace;
varying vec3 Normal_cameraspace;
varying vec3 EyeDirection_cameraspace;
varying vec3 LightDirection_cameraspace;


uniform mat4 mvpMatrix;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;
void main( void )
{
   // Output position of the vertex, in clip space : MVP * position
   gl_Position = mvpMatrix * vec4( v_position, 1.0 );

   // Position of the vertex, in worldspace : M * position
   Position_worldspace = (M * vec4(v_position,1)).xyz;

   // Vector that goes from the vertex to the camera, in camera space.
   // In camera space, the camera is at the origin (0,0,0).
   vec3 vertexPosition_cameraspace = ( V * M * vec4(v_position,1)).xyz;
   EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

   vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
   LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

   Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; 

   color = v_color;
}
