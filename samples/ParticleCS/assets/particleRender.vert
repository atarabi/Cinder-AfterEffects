#version 420
#extension GL_ARB_shader_storage_buffer_object : require

layout( location = 0 ) in int particleId;

struct Particle
{
	vec3	position;
	vec3	velocity;
	vec4	color;
	float	damping;
};

layout( std140, binding = 0 ) buffer Part
{
    Particle particles[];
};

out vec4 vColor;

void main()
{
	gl_Position = vec4( particles[particleId].position, 1.0 );
	vColor = particles[particleId].color;
}