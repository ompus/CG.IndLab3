#version 330 core

uniform float scale;
uniform sampler2D ourTexture;

in vec2 texcoord;
out vec4 color;

void main()
{
	float x = texcoord.x - 0.5;
	float y = texcoord.y - 0.5;
	if (((x*x + y*y) <= 0.04 * scale) && ((x*x + y*y) >= 0.02 * scale) || ((x*x + y*y) <= 0.25 * scale) && ((x*x + y*y) >= 0.16 * scale))
		color = vec4 (1.0, 0.0, 0.0, 1.0);
	else
		color = vec4 (0.0, 0.0, 0.0, 0.0);
}
