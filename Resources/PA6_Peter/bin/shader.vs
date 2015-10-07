#version 330

in vec3 v_position;
in vec2 v_texture;

out vec4 gl_Position;
out vec2 tex_coords;

uniform mat4 mvpMatrix;

void main(){
 gl_Position = mvpMatrix * vec4(v_position,1.0);
 tex_coords = v_texture;
}
