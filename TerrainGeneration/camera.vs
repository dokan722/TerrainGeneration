#version 330 core
layout (location = 0) in vec3 aPos;

out float height;
out float depth;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	height = aPos.y / 100;
	depth = gl_Position.z;
}