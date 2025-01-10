#version 330 core

uniform	vec4 lightAmbient;
uniform	vec4 lightDiffuse;
uniform	vec4 lightSpecular;

uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform vec4 materialEmission;
uniform float materialShininess;

in struct Vertex {
	vec2 texcoord;
	vec3 normal;
	vec3 lightDir;
	vec3 viewDir;
} Vert;

out vec4 color;

uniform sampler2D ourTexture;

void main()
{
	vec3 normal = normalize(Vert.normal);
	vec3 lightDir = normalize(Vert.lightDir);
	vec3 viewDir = normalize(Vert.viewDir);

	color = materialEmission;
	color += materialAmbient * lightAmbient;

	float Ndot = max(dot(normal, lightDir), 0.0);
	color += materialDiffuse * lightDiffuse * Ndot;

	float RdotVpow = max(pow (dot(reflect(-lightDir, normal), viewDir), materialShininess), 0.0);
	color += materialSpecular * lightSpecular * RdotVpow;

    color *= texture(ourTexture, Vert.texcoord);
}
