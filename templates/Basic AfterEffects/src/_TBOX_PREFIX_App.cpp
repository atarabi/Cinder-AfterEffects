#include "CinderAfterEffects.h"

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
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
	addParameter("Slider", 100.f);
}

void _TBOX_PREFIX_App::setupAE()
{
}

void _TBOX_PREFIX_App::updateAE()
{
}

void _TBOX_PREFIX_App::drawAE()
{
	gl::clear();
}

CINDER_APP_NATIVE(_TBOX_PREFIX_App, RendererGl)