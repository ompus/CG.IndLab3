#version 330 core

in vec2 TexCoord;
out vec4 color;

uniform sampler2D ourTexture;

void main()
{
	if (texture(ourTexture,TexCoord).x == 1.0 &&
	texture(ourTexture,TexCoord).y == 1.0 &&
	texture(ourTexture,TexCoord).z == 1.0 &&
	texture(ourTexture,TexCoord).a == 1.0)
		color = vec4 (0.0, 0.0, 0.0, 0.0);
	else
		color = texture(ourTexture,TexCoord);
}
