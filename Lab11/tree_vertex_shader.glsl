#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 matr;
uniform vec3 shift;
out vec2 TexCoord;

void main() {
	 gl_Position = matr * vec4(vec3(position[0], position[2], position[1]) / 120 + shift, 1.0f);
	 TexCoord = texCoord;
}