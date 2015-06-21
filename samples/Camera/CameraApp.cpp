#include "CinderAfterEffects.h"
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
	CameraAE camera_;
};

void CameraApp::initializeAE()
{
	addCameraParameter();
	addParameter("Position", Vec3f{0.5f, 0.5f, 0.f});
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
	gl::setMatrices(camera_);
}

void CameraApp::drawAE()
{
	gl::clear(ColorA(0, 0, 0, 0));
	Color color = getParameter("Color");
	gl::color(color);
	Vec3f position = getParameter("Position");
	float size = getParameter("Size");
	gl::drawCube(position, {size, size, size});
}

CINDER_APP_NATIVE(CameraApp, RendererGl)
