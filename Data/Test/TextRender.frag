#version 430
layout (location = 0) in vec2 inCoord;
layout (location = 0) out vec4 outFragColor;
uniform sampler2D fontTex;
void main()
{
  outFragColor = texture(fontTex, inCoord);
}
