#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 matr;
uniform vec3 shift;

uniform vec3 posView;
uniform	vec4 lightPosition;

out struct Vertex {
	vec2 texcoord;
	vec3 normal;
	vec3 lightDir;
	vec3 viewDir;
} Vert;

void main() {
	 vec3 pos = vec3(position[0], position[2], position[1]) + shift;
	 Vert.texcoord = texCoord;
	 Vert.normal = normal;
	 Vert.lightDir = vec3(lightPosition);
	 Vert.viewDir = posView - pos;
	 gl_Position = matr * vec4(pos, 1.0f);
}