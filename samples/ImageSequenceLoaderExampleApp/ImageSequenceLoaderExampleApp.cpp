#include "CinderAfterEffects.h"
#include "cinder/app/RendererGl.h"
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
	Color color_;
	
	ImageSequenceLoader loader_;
	gl::TextureRef texture_;
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
	color_ = getParameter("Color");

	if (!loader_.empty())
	{
		uint32_t frame = getCurrentFrame();
		texture_ = gl::Texture::create(loader_.getImage(frame));
	}
}

void ImageSequenceLoaderExampleApp::drawAE()
{
	gl::clear(ColorA(0, 0, 0, 0));
	gl::color(color_);
	gl::draw(texture_);
}

CINDER_APP(ImageSequenceLoaderExampleApp, RendererGl(RendererGl::Options().msaa(16)), [](App::Settings* settings)
{
	settings->setWindowSize(1280, 720);
	settings->setFrameRate(30.0f);
	settings->setResizable(false);
	settings->setFullScreen(false); 
})
