#include "CinderAfterEffects.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace atarabi;

class _TBOX_PREFIX_App : public AppAE {
public:
	void initializeAE() override;
	void setupAE() override;
	void updateAE() override;
	void drawAE() override;
};

void _TBOX_PREFIX_App::initializeAE()
{
	addParameter("Color", Color{1.f, 0.f, 0.f});
}

void _TBOX_PREFIX_App::setupAE()
{
}

void _TBOX_PREFIX_App::updateAE()
{
}

void _TBOX_PREFIX_App::drawAE()
{
	gl::clear(ColorA{ getParameter("Color") });
}

CINDER_APP(_TBOX_PREFIX_App, RendererGl(RendererGl::Options().msaa(16)), [](App::Settings* settings)
{
	settings->setWindowSize(1280, 720);
	settings->setFrameRate(30.0f);
	settings->setResizable(false);
	settings->setFullScreen(false);
})