#include "CinderAfterEffects.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace atarabi;

class ImageSequenceLoaderExampleApp : public AppAE {
public:
	void initializeAE() override;
	void setupAE() override;
	void updateAE() override;
	void drawAE() override;

private:
	ImageSequenceLoader loader_;
	gl::Texture texture_;
};

void ImageSequenceLoaderExampleApp::initializeAE()
{
	addParameter("Color", Color{ 1.f, 0.5f, 0.5f });
	loader_.setLoop(true);
}

void ImageSequenceLoaderExampleApp::setupAE()
{
	//the selected AV layer's source must be an image sequence.
	loader_.load(getSourcePath());
	texture_.reset();
}

void ImageSequenceLoaderExampleApp::updateAE()
{
	if (!loader_.empty())
	{
		uint32_t frame = getCurrentFrame();
		texture_ = loader_.getImage(frame);
	}
}

void ImageSequenceLoaderExampleApp::drawAE()
{
	gl::clear(Color(0, 0, 0));

	if (texture_)
	{
		Color color = getParameter("Color");
		gl::color(color);
		gl::draw(texture_);
	}
}

CINDER_APP_NATIVE(ImageSequenceLoaderExampleApp, RendererGl)
