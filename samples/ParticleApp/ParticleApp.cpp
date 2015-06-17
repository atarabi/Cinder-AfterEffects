#include "CinderAfterEffects.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Perlin.h"
#include "cinder/Color.h"

#include <vector>
#include <cmath>
#include <functional>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace atarabi;

class Particle {
public:
	Particle(float birth, float life, vec2 position, vec2 velocity, Color color, float size) : birth_{ birth }, life_{ life }, position_{ position }, velocity_{ velocity }, color_{ color }, size_{ size } {}

	bool update(float time, float frame_duration, std::function < vec2(vec2) > &get_accel)
	{
		if (time >= birth_ + life_)
		{
			return false;
		}

		vec2 accel = get_accel(position_);
		velocity_ += accel;
		position_ += velocity_;
		return true;
	}

	void draw(gl::BatchRef &batch, float time) const
	{
		float rate = 1.f - 2.f * std::abs(time - (birth_ + 0.5f * life_)) / life_;
		float size = size_ * std::sqrt(rate);

		gl::ScopedModelMatrix scoped_model_matrix;
		gl::ScopedColor scoped_color{ color_ };
		gl::translate(vec3{ position_, 0.f });
		gl::scale(vec3{ size });
		batch->draw();
	}

private:
	float birth_;
	float life_;
	vec2 position_;
	vec2 velocity_;
	ColorA color_;
	float size_;
};

class ParticleApp : public AppAE {
public:
	void initializeAE() override;
	void setupAE() override;
	void mouseMoveAE(MouseEvent event) override;
	void updateAE() override;
	void drawAE() override;

private:
	gl::BatchRef circle_;

	Perlin			perlin_;
	std::vector<Particle> particles_;
	vec2 prev_position_;
	vec2 position_;
};

void ParticleApp::initializeAE()
{
	//create a circle
	gl::GlslProgRef shader = gl::context()->getStockShader(gl::ShaderDef().color());
	circle_ = gl::Batch::create(geom::Circle().radius(1).subdivisions(60), shader);

	//add parameters
	addParameter("Number", 100.f);
	addParameter("Life", 1.f);
	addParameter("Emitter Radius", 10.f);
	addParameter("Inherit Velocity", -50.f);
	addParameter("Perlin Intensity", 100.f);
	addParameter("Color", Color{ 1.f, 0.f, 0.f });
	addParameter("Color Variance", 20.f);
	addParameter("Size", 5.f);
}

void ParticleApp::setupAE()
{
	perlin_.setSeed(0);
	particles_.clear();
	particles_.shrink_to_fit();
	gl::enableAlphaBlending();
	prev_position_ = position_ = 0.5f * vec2{ getSize() };
}

void ParticleApp::mouseMoveAE(MouseEvent event)
{
	position_ = event.getPos();
	if (getCurrentFrame() == 0)
	{
		prev_position_ = position_;
	}

	setParameter("Mouse", position_);
}

void ParticleApp::updateAE()
{
	auto current_frame = getCurrentFrame();

	//get parameters
	float number = getParameter("Number");

	float current_life = getParameter("Life");
	float next_life = getParameter("Life", current_frame + 1);
	float delta_life = next_life - current_life;

	float emitter_radius = std::max(0.1f, static_cast<float>(getParameter("Emitter Radius")));

	float inherit_velocity = static_cast<float>(getParameter("Inherit Velocity")) * 0.01f;

	float perlin_intensity = static_cast<float>(getParameter("Perlin Intensity")) * 0.01f;

	Color current_color = getParameter("Color");
	Color next_color = getParameter("Color", current_frame + 1);
	Color delta_color = next_color - current_color;

	float color_variance = static_cast<float>(getParameter("Color Variance")) * 0.01f;

	float current_size = std::max(0.f, static_cast<float>(getParameter("Size")));
	float next_size = std::max(0.f, static_cast<float>(getParameter("Size", current_frame + 1)));
	float delta_size = next_size - current_size;

	//create particles
	float time = getCurrentTime();
	float fps = getFps();
	float frame_duration = 1.f / fps;

	int births = static_cast<int>(number * frame_duration + 0.5f);
	vec2 delta_position = position_ - prev_position_;
	vec2 velocity = length(delta_position) > 0.f ? normalize(delta_position) * inherit_velocity : vec2{};

	for (int i : boost::irange(0, births))
	{
		float rate = static_cast<float>(i) / births;
		float birth = time + frame_duration * rate;
		float life = current_life + rate * delta_life;
		vec2 position = prev_position_ + rate * delta_position;
		Color color = current_color + rate * delta_color + Color(ColorModel::CM_RGB, Rand::randVec3() * color_variance);
		float size = current_size + rate * delta_size;
		particles_.emplace_back(birth, life, position + Rand::randVec2() * Rand::randFloat(emitter_radius), velocity, color, size);
	}

	//update particles
	Perlin &perlin = perlin_;
	std::function < vec2(vec2) > get_accel = [&perlin, time, perlin_intensity](vec2 position) -> vec2 {
		vec3 deriv = perlin.dfBm({ position.x, position.y, time * 0.01f }) * perlin_intensity;
		return{ deriv.x, deriv.y };
	};

	boost::remove_erase_if(particles_, [time, frame_duration, &get_accel](Particle &particle) -> bool {
		return !particle.update(time, frame_duration, get_accel);
	});

	prev_position_ = position_;
}

void ParticleApp::drawAE()
{
	gl::clear(ColorA(0, 0, 0, 0));
	gl::ScopedViewMatrix scoped_view_matrix;

	float time = getCurrentTime();

	for (const auto &particle : particles_)
	{
		particle.draw(circle_, time);
	}
}

CINDER_APP(ParticleApp, RendererGl(RendererGl::Options().msaa(16)), [](App::Settings* settings)
{
	settings->setWindowSize(1280, 720);
	settings->setFrameRate(30.0f);
	settings->setResizable(false);
	settings->setFullScreen(false);
})
