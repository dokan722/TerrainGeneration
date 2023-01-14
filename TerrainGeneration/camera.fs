#version 330 core
#define FOG 0
out vec4 FragColor;

in float height;
in float depth;

vec3 gradient(float h)
{
	if (h < -0.5)
		return mix(vec3(0, 0, 1), vec3(0, 1, 0), (h + 1) * 2);
	if (h < 0)
		return mix(vec3(0, 1, 0), vec3(1, 1, 0), (h + 0.5) * 2);
	if (h < 0.5)
		return mix(vec3(1, 1, 0), vec3(1, 0 ,0), h * 2);
	return mix(vec3(1, 0 ,0), vec3(0, 0, 0), (h - 0.5) *2);
}


void main()
{
#if FOG
	FragColor = mix(vec4(gradient(height), 1.0), vec4(0.2f, 0.3f, 0.3f, 1.0f), clamp((depth / 4), 0, 1));
#else
	FragColor = vec4(gradient(height), 1.0);
#endif
}