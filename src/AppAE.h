/*
*	The MIT License (MIT)
*
*	Copyright (c) 2015 Kareobana
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in
*	all copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*	THE SOFTWARE.
*/

#pragma once

#include "IAppAE.h"
#include "cinder/gl/Fbo.h"
#include "OscListener.h"
#include "OscSender.h"
#include "ImageWriter.h"
#include <map>

namespace atarabi {

/*
* AppAE
*/
class AppAE : public IAppAE {
public:
	static const int SERVER_PORT = 3000;
	static const int CLIENT_PORT = 3001;

	void prepareSettings(Settings *settings) final;
	void setup() final;
	void update() final;
	void draw() final;

	void mouseDown(cinder::app::MouseEvent event) final;
	void mouseUp(cinder::app::MouseEvent event) final;
	void mouseWheel(cinder::app::MouseEvent event) final;
	void mouseMove(cinder::app::MouseEvent event) final;
	void mouseDrag(cinder::app::MouseEvent event) final;

protected:
	float getFps() const override { return mFps; }
	uint32_t getDuration() const override { return mDuration; }
	uint32_t getCurrentFrame() const override { return mCurrentFrame; }
	float getCurrentTime() const override { return mCurrentFrame / mFps; }

	int getWidth() const override { return mWidth; }
	int getHeight() const override { return mHeight; }
	cinder::Vec2i getSize() const override { return useFbo() ? cinder::Vec2i{ getWidth(), getHeight() } : getWindowSize(); };

	std::string getSourcePath() const override { return mSourcePath; }
	float getSourceTime() const override { return mSourceTime; }

	using IAppAE::addParameter;
	void addParameter(const std::string &name, ParameterType type, ParameterValue initialValue) override;

	using IAppAE::getParameter;
	ParameterValue getParameter(const std::string &name, uint32_t frame) const override;

	using IAppAE::getCameraParameter;
	CameraAE::Parameter getCameraParameter(uint32_t frame) const override;

	using IAppAE::setParameter;
	void setParameter(const std::string &name, ParameterType type, ParameterValue value, uint32_t frame) override;

	void setCameraParameter(const cinder::Camera &camera) override;

	bool useFbo() const override;

private:
	enum class State {
		Uninitialized,
		Setup,
		Render,
		Setdown
	};

	enum SetupArg {
		SETUP_ARG_PATH,
		SETUP_ARG_FILENAME,
		SETUP_ARG_CACHE,
		SETUP_ARG_WRITE,
		SETUP_ARG_FBO,
		SETUP_ARG_FPS,
		SETUP_ARG_DURATION,
		SETUP_ARG_WIDTH,
		SETUP_ARG_HEIGHT,
		SETUP_ARG_SOURCE,
		SETUP_ARG_SOURCETIME
	};

	struct Getter {
		using Value = ParameterValue;
		uint32_t id;
		std::string name;
		ParameterType type;
		ParameterValue initialValue;
		std::vector<Value> values;
	};

	struct Setter {
		using Value = std::pair<int32_t, ParameterValue>;
		uint32_t id;
		std::string name;
		ParameterType type;
		std::vector<Value> values;
	};

	bool isParameterCached() const;
	void transition(State state);
	void setdown();
	void processMessage();
	void processSetupMessage(const cinder::osc::Message &message, const std::vector<std::string> &paths);
	void processPrerenderMessage(const cinder::osc::Message &message, const std::vector<std::string> &paths);
	void writeImage();

	State mState = State::Uninitialized;
	uint32_t mCurrentFrame = 0;

	cinder::gl::Fbo mMultisampleFbo;
	cinder::gl::Fbo mNormalFbo;

	std::vector<CameraAE::Parameter> mCameraGetters;
	std::vector<std::pair<int32_t, CameraAE::Parameter>> mCameraSetters;
	std::map<std::string, Getter> mGetters;
	std::map<std::string, Setter> mSetters;

	cinder::osc::Sender mSender;
	cinder::osc::Listener mListener;
	ImageWriter mWriter;

	//from AE
	std::string mPath;
	std::string mFileName = "cinder";
	bool mCache = false;
	bool mWrite = false;
	bool mUseFbo = false;
	float mFps = 30.f;
	uint32_t mDuration = 1;
	int mWidth = 1;
	int mHeight = 1;
	std::string mSourcePath;
	float mSourceTime = 0.f;
};

}