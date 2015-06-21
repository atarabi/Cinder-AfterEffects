#include "CinderAfterEffects.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Ssbo.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace atarabi;

#pragma pack( push, 1 )
struct Particle {
	vec3	position;
	float   pad1;
	vec3	velocity;
	float   pad2;
	vec4    color;
	float	damping;
	vec3    pad3;
};
#pragma pack( pop )

class ParticleCSApp : public AppAE {
	static const int PARTICLE_GRID = 50;
	static const int PARTICLE_NUM = PARTICLE_GRID * PARTICLE_GRID * PARTICLE_GRID;

public:
	void initializeAE() override;
	void setupAE() override;
	void updateAE() override;
	void drawAE() override;

private:
	CameraAE camera_;

	// gl
	static const int WORK_GROUP_SIZE = 128;

	gl::GlslProgRef render_program_;
	gl::GlslProgRef update_program_;

	gl::SsboRef particle_buffer_;
	gl::VboRef id_buffer_;
	gl::VaoRef vao_;
};

void ParticleCSApp::initializeAE()
{
	// add parameters
	addCameraParameter();
	addParameter("Force Position 1", vec3{ 0.25f, 0.5f, 0.f });
	addParameter("Force 1", 100.f);
	addParameter("Force Position 2", vec3{ 0.75f, 0.5f, 0.f });
	addParameter("Force 2", -100.f);
	addParameter("Circle Radius", 3.f);

	addParameter("Center", vec3{ 0.5f, 0.5f, 0.f });
	addParameter("Size", 5000.f);
	addParameter("Damping", 95.f);
	addParameter("Color", Color{ 1.f, 0.f, 0.f });
	addParameter("Opacity", 100.f);

	// setup gl
	gl::enableAlphaBlending();
	gl::context()->blendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // screen for premultiplied
	setUnmultiply(true);

	ivec3 count = gl::getMaxComputeWorkGroupCount();
	CI_ASSERT(count.x >= (PARTICLE_NUM / WORK_GROUP_SIZE));

	try {
		render_program_ = gl::GlslProg::create(gl::GlslProg::Format{}
			.vertex(loadAsset("particleRender.vert"))
			.geometry(loadAsset("particleRender.geom"))
			.fragment(loadAsset("particleRender.frag"))
			.attribLocation("particleId", 0));
	}
	catch (gl::GlslProgCompileExc e) {
		console() << e.what() << std::endl;
		quit();
	}

	try {
		update_program_ = gl::GlslProg::create(gl::GlslProg::Format{}
			.compute(loadAsset("particleUpdate.comp")));
	}
	catch (gl::GlslProgCompileExc e) {
		console() << e.what() << std::endl;
		quit();
	}

	particle_buffer_ = gl::Ssbo::create(PARTICLE_NUM * sizeof(Particle));
	gl::ScopedBuffer scopedParticleSsbo(particle_buffer_);
	particle_buffer_->bindBase(0);

	std::vector<GLuint> ids(PARTICLE_NUM);
	GLuint id = 0;
	std::generate(ids.begin(), ids.end(), [&id]() -> GLuint { return id++; });
	id_buffer_ = gl::Vbo::create<GLuint>(GL_ARRAY_BUFFER, ids, GL_STATIC_DRAW);
	vao_ = gl::Vao::create();
	{
		gl::ScopedVao vao(vao_);
		gl::ScopedBuffer buffer(id_buffer_);
		gl::enableVertexAttribArray(0);
		gl::vertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(GLuint), 0);
	}
	
}

void ParticleCSApp::setupAE()
{
	// set camera
	camera_.setPerspective(30.f, static_cast<float>(getWidth()) / static_cast<float>(getHeight()), 1.f, 5000.f);

	// get parameters
	vec3 center = getParameter("Center", 0);
	float size = getParameter("Size", 0);
	Color color = getParameter("Color", 0);
	vec3 hsv = rgbToHsv(color);
	float damping = getParameter("Damping", 0);
	damping = math<float>::clamp(damping * 0.01f, 0.f, 1.f);
	float opacity = getParameter("Opacity", 0);
	opacity = math<float>::clamp(opacity * 0.01f, 0.f, 1.f);

	console() << "Damping: " << damping << std::endl;
	console() << "Opacity: " << opacity << std::endl;

	// setup particles
	std::vector<Particle> particles;
	particles.assign(PARTICLE_NUM, Particle());

	Rand rand{ 0 };
	for (int i = 0; i < PARTICLE_GRID; ++i)
	{
		float x = size * i / (PARTICLE_GRID - 1) - 0.5f * size;
		for (int j = 0; j < PARTICLE_GRID; ++j)
		{
			float y = size * j / (PARTICLE_GRID - 1) - 0.5f * size;
			for (int k = 0; k < PARTICLE_GRID; ++k)
			{
				float z = size * k / (PARTICLE_GRID - 1) - 0.5f * size;

				auto &particle = particles[i * PARTICLE_GRID * PARTICLE_GRID + j * PARTICLE_GRID + k];
				particle.position = center + vec3{ x, y, z };
				particle.velocity = vec3{};
				particle.damping = damping;
				vec3 c = hsvToRgb(hsv + vec3{ rand.randFloat(), 0.f, 0.f } * 0.1f);
				particle.color = vec4{ c, opacity };
			}
		}
	}

	particle_buffer_->bufferSubData(0, PARTICLE_NUM * sizeof(Particle), particles.data());
}

void ParticleCSApp::updateAE()
{
	// update parameters
	camera_.setParameter(getCameraParameter());
	vec3 position1 = getParameter("Force Position 1");
	float force1 = getParameter("Force 1");
	vec3 position2 = getParameter("Force Position 2");
	float force2 = getParameter("Force 2");

	// update particles
	gl::ScopedGlslProg program(update_program_);

	update_program_->uniform("uPosition1", position1);
	update_program_->uniform("uForce1", force1);
	update_program_->uniform("uPosition2", position2);
	update_program_->uniform("uForce2", force2);
	update_program_->uniform("uFps", getFps());
	gl::ScopedBuffer scopedParticleSsbo(particle_buffer_);

	gl::dispatchCompute(PARTICLE_NUM / WORK_GROUP_SIZE, 1, 1);
	gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ParticleCSApp::drawAE()
{
	gl::clear(ColorA{ 0.f, 0.f, 0.f, 0.f });
	gl::ScopedGlslProg program(render_program_);
	gl::ScopedBuffer buffer(particle_buffer_);
	gl::ScopedVao vao(vao_);

	gl::setMatrices(camera_);
	gl::context()->setDefaultShaderVars();

	vec3 right, up;
	camera_.getBillboardVectors(&right, &up);
	float radius = math<float>::max(0.f, getParameter("Circle Radius"));
	render_program_->uniform("uRight", right);
	render_program_->uniform("uUp", up);
	render_program_->uniform("uRadius", radius);

	gl::drawArrays(GL_POINTS, 0, PARTICLE_NUM);
}

CINDER_APP(ParticleCSApp, RendererGl(RendererGl::Options().msaa(16)), [](App::Settings* settings)
{
	settings->setWindowSize(1280, 720);
	settings->setFrameRate(30.0f);
	settings->setResizable(false);
	settings->setFullScreen(false);
})
