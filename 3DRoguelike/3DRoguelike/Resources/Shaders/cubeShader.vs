#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 instanceOffset;
layout (location = 3) in vec3 instanceColor;
layout (location = 4) in float instanceScale;

out vec2 TexCoord;
out vec3 Color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * vec4(instanceScale * aPos + instanceOffset, 1.0f);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	Color = instanceColor;
}
