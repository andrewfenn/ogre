#version 150

uniform sampler2D RT;
in vec2 uv;
out vec4 fragColour;

void main()
{
    vec3 greyscale = vec3(dot(texture(RT, uv).rgb, vec3(0.3, 0.59, 0.11)));
	fragColour = vec4(greyscale, 1.0);
}
