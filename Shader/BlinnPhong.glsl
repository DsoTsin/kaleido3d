#version 430

#ifdef VERTEX_SHADER
// model global uniform matrix
uniform mat4 gProj;
uniform mat4 gWorld;
uniform mat4 gView;

layout(location = 0) in vec3 position;

#ifdef USE_NORMMAP
uniform sampler2D	normMap;
#define NORMALVAL	(texture(normMap, texcoord).rgb);
#else
layout(location = 1) in vec3 normal;
#define NORMAVAL	(normal)
#endif

#if defined(USE_DIFFMAP) || defined(USE_SPECMAP) || defined(USE_AMBMAP) || defined(USE_NORMMAP)
layout(location = 2) in vec2 texcoord;
out vec2 sTexCoord;
#endif 

// view space light position
uniform vec3 lightPosition = vec3(20.0, 20.0, 20.0);

// out varyings
out vec3 sNormal;
out vec3 sLightDir;
out vec3 sViewDir;

void main()
{
	mat4 vm 		= gView*gWorld;
	vec4 vPosition 	= vm*vec4(position, 1.0);
	sNormal 		= mat3(vm)* NORMAVAL ;
	sLightDir 		= lightPosition - vPosition.xyz;
	sViewDir		= -vPosition.xyz;
	
#if defined(USE_DIFFMAP) || defined(USE_SPECMAP) || defined(USE_AMBMAP)
	sTexCoord 		= texcoord;
#endif 
	gl_Position 	= gProj*vPosition;
}

#endif


#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 FragColor;
in vec3 sNormal;
in vec3 sLightDir;
in vec3 sViewDir;

#if defined(USE_DIFFMAP) || defined(USE_SPECMAP) || defined(USE_AMBMAP)
in vec2 sTexCoord;
#endif 

#ifdef USE_DIFFMAP
uniform sampler2D 	albedoTexture;
#define DIFFCOLOR 	(texture(albedoTexture, sTexCoord).rgb)
#else
uniform vec3		albedoColor = vec3(0.5, 0.2, 0.7);
#define DIFFCOLOR 	(albedoColor)
#endif

#ifdef USE_SPECMAP
uniform sampler2D	specTexture;
#define SPECCOLOR 	(texture(specTexture, sTexCoord).rgb)
#else
uniform vec3		specColor = vec3(0.7);
#define SPECCOLOR 	(specColor)
#endif

#ifdef USE_AMBMAP
uniform sampler2D 	ambtTexture;
#define AMBCOLOR	(texture(ambtTexture, sTexCoord).rgb)
#else
uniform vec3 		ambtColor = vec3(0.1, 0.1, 0.1);
#define AMBCOLOR	(ambtColor)
#endif 

#ifdef USE_ROGHMAP
uniform sampler2D	roghTexture;
#define ROGHCOEFF	(texture(roghTexture, sTexCoord).r)
#else
uniform float		roghValue = 20.0f;
#define ROGHCOEFF	(roghValue)
#endif

vec4 blinn_phong(in vec3 N, in vec3 L, in vec3 V, in vec3 diff, in vec3 spec, in vec3 amb) 
{
	vec3 H = normalize(L+V);
	vec3 diffuse = max(dot(N,L), 0.0) * diff;
	vec3 specular = pow(max(dot(N,H),0.0), ROGHCOEFF) * spc;
	return vec4(diffuse+specular+ amb, 1.0);
}

void main()
{
	vec3 N = normalize(sNormal);
	vec3 V = normalize(sViewDir);
	vec3 L = normalize(sLightDir);
#ifdef MASKEDIFF
	vec4 diff = vec4(0);
	diff = texture(ambtTexture, sTexCood);
	if(diff.a < 0.1)
		discard;
	else {
		FragColor = blinn_phong(N,L,V, diff.rgb, SPECCOLOR, AMBCOLOR);
	}
#else
	FragColor = blinn_phong(N,L,V, DIFFCOLOR, SPECCOLOR, AMBCOLOR);
#endif
}

#endif