#version 330

in vec2 tex_coords;
uniform sampler2D gSampler;
out vec4 color;

void main()
{
 color = texture(gSampler,tex_coords.xy);
}
