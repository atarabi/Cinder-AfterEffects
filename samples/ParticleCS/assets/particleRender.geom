#version 420

layout( points ) in;
layout( triangle_strip, max_vertices = 4 ) out;

uniform mat4 ciModelViewProjection;

uniform vec3 uUp;
uniform vec3 uRight;
uniform float uRadius;

in vec4 vColor[];

flat out vec4 gColor;
flat out vec3 gCenter;
out vec3 gPosition;

void main()
{
	vec3 center = gl_in[0].gl_Position.xyz;
	vec4 color = vColor[0];

	vec3 leftUp = center + uRadius * ( uUp - uRight );
	vec3 rightUp = center + uRadius * ( uUp + uRight );
	vec3 leftBottom = center + uRadius * ( -uUp - uRight );
	vec3 rightBottom = center + uRadius * ( -uUp + uRight );

	//emit
	gColor = color;
	gCenter = center;
	gPosition = leftUp;
	gl_Position = ciModelViewProjection * vec4( leftUp, 1.0 );
	EmitVertex();

	gColor = color;
	gCenter = center;
	gPosition = rightUp;
	gl_Position = ciModelViewProjection * vec4( rightUp, 1.0 );
	EmitVertex();

	gColor = color;
	gCenter = center;
	gPosition = leftBottom;
	gl_Position = ciModelViewProjection * vec4( leftBottom, 1.0 );
	EmitVertex();

	gColor = color;
	gCenter = center;
	gPosition = rightBottom;
	gl_Position = ciModelViewProjection * vec4( rightBottom, 1.0 );
	EmitVertex();
	EndPrimitive();
}
