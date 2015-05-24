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

#include "AppAEdev.h"
#include <functional>

namespace atarabi {

void AppAEdev::prepareSettings(Settings *settings)
{
	settings->setWindowSize(mWidth, mHeight);
	settings->setFrameRate(mFps);
	settings->setResizable(false);
	settings->setFullScreen(false);
}

void AppAEdev::setup()
{
	mParams = cinder::params::InterfaceGl::create("Parameters", cinder::Vec2i(200, 160));

	initializeAE();

	setupParams();

	timelineAE().stepTo(0.f);
	setupAE();
}

void AppAEdev::setupParams()
{
	mParams->addSeparator("Settings");
	mParams->addParam("Pause", &mPause);
	mParams->addButton("Restart", [this]() -> void {
		mCurrentFrame = 0;
		timelineAE().clear();
		timelineAE().stepTo(0.f);
		setupAE();
	});

	if (mUseCamera)
	{
		mFov = 22.9f;
		mEye = cinder::Vec3f{ 0.5f * getWidth(), 0.5f * getHeight(), -1777.7778f };
		mTarget = cinder::Vec3f{ 0.5f * getWidth(), 0.5f * getHeight(), 0.f };
		mUp = -cinder::Vec3f::yAxis();
		updateCamera();

		std::function<void()> updateCamera = std::bind(&AppAEdev::updateCamera, this);

		mParams->addSeparator("Camera");
		mParams->addParam("Fov", &mFov).min(0.1f).max(180.f).updateFn(updateCamera);
		mParams->addParam("Eye x", &mEye.x).updateFn(updateCamera);
		mParams->addParam("Eye y", &mEye.y).updateFn(updateCamera);
		mParams->addParam("Eye z", &mEye.z).updateFn(updateCamera);
		mParams->addParam("Target x", &mTarget.x).updateFn(updateCamera);
		mParams->addParam("Target y", &mTarget.y).updateFn(updateCamera);
		mParams->addParam("Target z", &mTarget.z).updateFn(updateCamera);
		mParams->addParam("Up", &mUp).updateFn(updateCamera);
		mParams->addSeparator();
	}

	mParams->addParam("Frame", &mCurrentFrame, true);
	mParams->addParam("Duration", &mDuration);
	mParams->addParam("Width", &mWidth).min(320.f).updateFn([this]() -> void {
		setWindowSize(mWidth, mHeight);
	});
	mParams->addParam("Height", &mHeight).min(240.f).updateFn([this]() -> void {
		setWindowSize(mWidth, mHeight);
	});
	mParams->addParam("Fps", &mFps).min(1.f).max(60.f).updateFn([this]() -> void {
		setFrameRate(mFps);
	});
	mParams->addParam("SourcePath", &mSourcePath, true);
	mParams->addButton("Open", [this]() -> void {
		auto path = cinder::app::getOpenFilePath("", { "mov", "avi", "mp4", "jpg", "jpeg", "png", "gif", "tif", "tiff", "bmp" }).string();
		if (!path.empty())
		{
			mSourcePath = path;
		}
	});
	mParams->addParam("SourceTime", &mSourceTime).min(0.f);
}

void AppAEdev::updateCamera()
{
	mCamera.setPerspective(mFov, static_cast<float>(getWidth()) / static_cast<float>(getHeight()), 5.f, 5000.f);
	mCamera.lookAt(mEye, mTarget, mUp);
}

void AppAEdev::update()
{
	if (!mPause)
	{
		timelineAE().stepTo(getCurrentTime());
		updateAE();
	}
}

void AppAEdev::draw()
{
	if (!mPause)
	{
		drawAE();
	}

	mParams->draw();

	if (!mPause)
	{
		++mCurrentFrame;
	}
}

void AppAEdev::mouseDown(cinder::app::MouseEvent event)
{
	mouseDownAE(event);
}

void AppAEdev::mouseUp(cinder::app::MouseEvent event)
{
	mouseUpAE(event);
}

void AppAEdev::mouseWheel(cinder::app::MouseEvent event)
{
	mouseWheelAE(event);
}

void AppAEdev::mouseMove(cinder::app::MouseEvent event)
{
	mouseMoveAE(event);
}

void AppAEdev::mouseDrag(cinder::app::MouseEvent event)
{
	mouseDragAE(event);
}

void AppAEdev::addParameter(const std::string &name, ParameterType type, ParameterValue initialValue)
{
	if (mParameters.count(name) == 0)
	{
		const auto &pair = mParameters.insert(std::make_pair(name, Parameter{ name, type }));
		auto &parameter = (*pair.first).second;
		switch (type)
		{
			case ParameterType::Checkbox:
				parameter.value.checkbox = initialValue;
				mParams->addParam(name, &parameter.value.checkbox);
				break;
			case ParameterType::Slider:
				parameter.value.slider = initialValue;
				mParams->addParam(name, &parameter.value.slider);
				break;
			case ParameterType::Point:
				parameter.value.point = initialValue;
				parameter.value.point *= cinder::Vec2f{ static_cast<float>(DEFAULT_WIDTH), static_cast<float>(DEFAULT_HEIGHT) };
				mParams->addParam(name + " x", &parameter.value.point.x);
				mParams->addParam(name + " y", &parameter.value.point.y);
				break;
			case ParameterType::Point3D:
				parameter.value.point3d = initialValue;
				parameter.value.point3d *= cinder::Vec3f{ static_cast<float>(DEFAULT_WIDTH), static_cast<float>(DEFAULT_HEIGHT), static_cast<float>(DEFAULT_WIDTH) };
				mParams->addParam(name + " x", &parameter.value.point3d.x);
				mParams->addParam(name + " y", &parameter.value.point3d.y);
				mParams->addParam(name + " z", &parameter.value.point3d.z);
				break;
			case ParameterType::Color:
				parameter.value.color = initialValue;
				mParams->addParam(name, &parameter.value.color);
				break;
		}
	}
}

ParameterValue AppAEdev::getParameter(const std::string &name, uint32_t) const
{
	assert(mParameters.count(name) > 0);

	const auto it = mParameters.find(name);
	const auto &parameter = it->second;

	switch (parameter.type)
	{
		case ParameterType::Checkbox:
			return parameter.value.checkbox;
		case ParameterType::Slider:
			return parameter.value.slider;
		case ParameterType::Point:
			return parameter.value.point;
		case ParameterType::Point3D:
			return parameter.value.point3d;
		case ParameterType::Color:
			return parameter.value.color;
	}
}

CameraAE::Parameter AppAEdev::getCameraParameter(uint32_t) const
{
	float fov = mCamera.getFov();
	cinder::Matrix44f cameraMatrix = mCamera.getModelViewMatrix().affineInverted();
	cameraMatrix.scale(cinder::Vec3f{ 1.f, -1.f, -1.f });
	return{ fov, cameraMatrix };
}

}