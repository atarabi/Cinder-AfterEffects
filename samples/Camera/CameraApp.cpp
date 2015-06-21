#include "CinderAfterEffects.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace atarabi;

class CameraApp : public AppAE {
public:
	void initializeAE() override;
	void setupAE() override;
	void updateAE() override;
	void drawAE() override;

private:
	gl::BatchRef sphere_;

	CameraAE camera_;
	Color color_;
	vec3 position_;
	float size_;
};

void CameraApp::initializeAE()
{
	//create a sphere
	gl::GlslProgRef shader = gl::context()->getStockShader(gl::ShaderDef().color());
	sphere_ = gl::Batch::create(geom::Sphere().radius(1.f).subdivisions(60), shader);

	//add parameters
	addCameraParameter();
	addParameter("Position", vec3{ 0.5f, 0.5f, 0.f });
	addParameter("Color", Color{ 1.f, 0.f, 0.f });
	addParameter("Size", 50.f);
}

void CameraApp::setupAE()
{
	camera_.setPerspective(60.f, static_cast<float>(getWidth()) / static_cast<float>(getHeight()), 5.f, 5000.f);
}

void CameraApp::updateAE()
{
	camera_.setParameter(getCameraParameter());
	position_ = getParameter("Position");
	color_ = getParameter("Color");
	size_ = getParameter("Size");
}

void CameraApp::drawAE()
{
	gl::clear(ColorA(0, 0, 0, 0));

	gl::ScopedViewMatrix scoped_view_matrix;
	gl::setMatrices(camera_);

	gl::ScopedColor scoped_color{ color_ };

	gl::ScopedModelMatrix scoped_model_matrix;
	gl::translate(position_);
	gl::scale(vec3{ size_ });

	sphere_->draw();
}

CINDER_APP(CameraApp, RendererGl(RendererGl::Options().msaa(16)), [](App::Settings* settings)
{
	settings->setWindowSize(1280, 720);
	settings->setFrameRate(30.0f);
	settings->setResizable(false);
	settings->setFullScreen(false); 
})
