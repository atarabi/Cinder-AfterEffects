#version 420

uniform float uRadius;

flat in vec4 gColor;
flat in vec3 gCenter;
in vec3 gPosition;

out vec4 outColor;

void main()
{
	float radius = length(gPosition - gCenter);
	vec4 color = gColor;
	color.a *= 1.0 - smoothstep(uRadius - 1.0, uRadius, radius);
	color.rgb *= color.a;
    outColor = color;
}