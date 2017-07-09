#version 430
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inCoord;
layout (location = 0) out vec2 outCoord;
out gl_PerVertex 
{
    vec4 gl_Position;   
};
void main() 
{
	outCoord = inCoord;
	gl_Position = vec4(inPos, 1.0);
}