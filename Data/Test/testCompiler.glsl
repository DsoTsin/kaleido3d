#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D tex;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 vertColor;
layout (location = 2) in vec2 vertUV;

layout (location = 0) out vec4 color;
layout (location = 1) out vec2 uv;

void main() {
   color = vertColor;
   uv = vertUV + texture(tex, vertUV).xy;
   gl_Position  = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(pos, 1.0f);
}