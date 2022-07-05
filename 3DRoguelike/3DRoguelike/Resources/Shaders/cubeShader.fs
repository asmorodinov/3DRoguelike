#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Color;

uniform sampler2D texture1;

void main()
{
	FragColor = vec4(texture(texture1, TexCoord).rgb * Color, 1.0);
}
