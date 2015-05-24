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
#include "cinder/params/Params.h"
#include <map>

namespace atarabi {

/*
* AppAE for development
*/
class AppAEdev : public IAppAE {
private:
	static const int DEFAULT_WIDTH = 1280;
	static const int DEFAULT_HEIGHT = 720;

	struct Parameter {
		struct Value {
			bool checkbox;
			float slider;
			cinder::Vec2f point;
			cinder::Vec3f point3d;
			cinder::Color color;
		};
		std::string name;
		ParameterType type;
		Value value;
	};

public:
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
	uint32_t getDuration() const override { return std::max(mCurrentFrame, mDuration); }
	uint32_t getCurrentFrame() const override { return mCurrentFrame; }
	float getCurrentTime() const override { return mCurrentFrame / mFps; }

	int getWidth() const override { return getWindowWidth(); }
	int getHeight() const override { return getWindowHeight(); }
	cinder::Vec2i getSize() const override { return getWindowSize(); };

	std::string getSourcePath() const override { return mSourcePath; }
	float getSourceTime() const override { return mSourceTime; }

	using IAppAE::addParameter;
	void addParameter(const std::string &name, ParameterType type, ParameterValue initialValue) override;

	using IAppAE::getParameter;
	// !Same as getParameter(const std::string &).
	ParameterValue getParameter(const std::string &name, uint32_t /* frame */) const override;

	using IAppAE::getCameraParameter;
	// !Same as getCameraParameter().
	CameraAE::Parameter getCameraParameter(uint32_t /* frame */) const override;

private:
	void setupParams();
	void updateCamera();

	bool mPause = false;
	uint32_t mCurrentFrame = 0;

	cinder::CameraPersp mCamera;
	float mFov;
	cinder::Vec3f mEye;
	cinder::Vec3f mTarget;
	cinder::Vec3f mUp;
	cinder::params::InterfaceGlRef mParams;
	std::map<std::string, Parameter> mParameters;

	//from AE
	uint32_t mDuration = 900;
	float mFps = 30.f;
	int mWidth = DEFAULT_WIDTH;
	int mHeight = DEFAULT_HEIGHT;
	std::string mSourcePath = "";
	float mSourceTime = 0.f;
};

}