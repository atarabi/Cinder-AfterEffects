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

#include "cinder/app/AppNative.h"
#include "cinder/Timeline.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include <string>
#include <vector>

#include "CameraAE.h"

namespace atarabi {

/*
* Parameter
*/
enum class ParameterType {
	Checkbox,
	Slider,
	Point,
	Point3D,
	Color
};

union ParameterValue {
	struct {
		bool value;
	} checkbox;

	struct {
		float value;
	} slider;

	struct {
		float x, y;
	} point;

	struct {
		float x, y, z;
	} point3d;

	struct {
		float r, g, b;
	} color;

	ParameterValue() {}

	ParameterValue(bool value)
	{
		checkbox.value = value;
	}

	operator bool() const
	{
		return checkbox.value;
	}

	ParameterValue(float value)
	{
		slider.value = value;
	}

	operator float() const
	{
		return slider.value;
	}

	ParameterValue(cinder::Vec2f value)
	{
		point.x = value.x;
		point.y = value.y;
	}

	operator cinder::Vec2f() const
	{
		return{ point.x, point.y };
	}

	ParameterValue(const cinder::Vec3f& value)
	{
		point3d.x = value.x;
		point3d.y = value.y;
		point3d.z = value.z;
	}

	operator cinder::Vec3f() const
	{
		return{ point3d.x, point3d.y, point3d.z };
	}

	ParameterValue(const cinder::Color& value)
	{
		color.r = value.r;
		color.g = value.g;
		color.b = value.b;
	}

	operator cinder::Color() const
	{
		return{ color.r, color.g, color.b };
	}
};

/*
* AppAE Interface Class
*/
class IAppAE : public cinder::app::AppNative {
public:
	IAppAE() : mTimelineAE{ cinder::Timeline::create() } {}

	//! Override to perform any application setup.
	virtual void initializeAE() {}
	//! Override to perform any rendering setup.
	virtual void setupAE() {}
	//! Override to perform any once-per-loop computation.
	virtual void updateAE() {}
	//! Override to perform any rendering once-per-loop.
	virtual void drawAE() {}

	//! Override to receive mouse-down events.
	virtual void mouseDownAE(cinder::app::MouseEvent event) {}
	//! Override to receive mouse-up events.
	virtual void mouseUpAE(cinder::app::MouseEvent event) {}
	//! Override to receive mouse-wheel events.
	virtual void mouseWheelAE(cinder::app::MouseEvent event) {}
	//! Override to receive mouse-move events.
	virtual void mouseMoveAE(cinder::app::MouseEvent event) {}
	//! Override to receive mouse-drag events.
	virtual void mouseDragAE(cinder::app::MouseEvent event) {}

	//! Returns a reference to the AppAE's Timeline
	cinder::Timeline& timelineAE() { return *mTimelineAE; }

protected:
	//! Returns the fps of the composition.
	virtual float getFps() const = 0;
	//! Returns the duration of the layer.
	virtual uint32_t getDuration() const = 0;
	//! Returns the number of frames.
	virtual uint32_t getCurrentFrame() const = 0;
	//! Returns the number of seconds.
	virtual float getCurrentTime() const = 0;

	//! Returns the width of the layer.
	virtual int getWidth() const = 0;
	//! Returns the height of the layer.
	virtual int getHeight() const = 0;
	//! Returns the size of the layer.
	virtual cinder::Vec2i getSize() const = 0;

	//! Returns the path of the selected AV layer's source(it can be empty).
	virtual std::string getSourcePath() const = 0;
	//! Returns the start time of the selected AV layer's source.
	virtual float getSourceTime() const = 0;

	//! Adds a parameter(control effect) to After Effects(must be called in initializeAE()).
	virtual void addParameter(const std::string &name, ParameterType type, ParameterValue initialValue) = 0;
	void addParameter(const std::string &name, bool initialValue);
	void addParameter(const std::string &name, float initialValue); 
	void addParameter(const std::string &name, cinder::Vec2f initialValue);
	void addParameter(const std::string &name, cinder::Vec3f initialValue);
	void addParameter(const std::string &name, cinder::Color initialValue);

	//! Adds a "CameraAE" plugin to AfterEffects to get information about the camera used in AfterEffects.
	void addCameraParameter() { mUseCamera = true; }

	//! Returns the value of the added parameter.
	virtual ParameterValue getParameter(const std::string &name, uint32_t frame) const = 0;
	ParameterValue getParameter(const std::string &name) const;

	//! Returns the value of the "CamerAE" plugin.
	virtual CameraAE::Parameter getCameraParameter(uint32_t frame) const = 0;
	CameraAE::Parameter getCameraParameter() const;

	//! Sets the value of the parameter which will be baked in After Effects after rendering images.
	virtual void setParameter(const std::string &name, ParameterType type, ParameterValue value) {}
	void setParameter(const std::string &name, bool value);
	void setParameter(const std::string &name, float value);
	void setParameter(const std::string &name, cinder::Vec2f value);
	void setParameter(const std::string &name, cinder::Vec3f value);
	void setParameter(const std::string &name, cinder::Color value);

	//! Returns whether to use a fbo when rendering images.
	virtual bool useFbo() const  { return false; }

protected:
	bool mUseCamera = false;

private:
	std::shared_ptr<cinder::Timeline> mTimelineAE;

};

inline void IAppAE::addParameter(const std::string &name, bool initialValue)
{
	addParameter(name, ParameterType::Checkbox, initialValue);
}

inline void IAppAE::addParameter(const std::string &name, float initialValue)
{
	addParameter(name, ParameterType::Slider, initialValue);
}

inline void IAppAE::addParameter(const std::string &name, cinder::Vec2f initialValue)
{
	addParameter(name, ParameterType::Point, initialValue);
}

inline void IAppAE::addParameter(const std::string &name, cinder::Vec3f initialValue)
{
	addParameter(name, ParameterType::Point3D, initialValue);
}

inline void IAppAE::addParameter(const std::string &name, cinder::Color initialValue)
{
	addParameter(name, ParameterType::Color, initialValue);
}

inline ParameterValue IAppAE::getParameter(const std::string &name) const
{
	return getParameter(name, getCurrentFrame());
}

inline CameraAE::Parameter IAppAE::getCameraParameter() const
{
	return getCameraParameter(getCurrentFrame());
}

inline void IAppAE::setParameter(const std::string &name, bool value)
{
	setParameter(name, ParameterType::Checkbox, value);
}

inline void IAppAE::setParameter(const std::string &name, float value)
{
	setParameter(name, ParameterType::Slider, value);
}

inline void IAppAE::setParameter(const std::string &name, cinder::Vec2f value)
{
	setParameter(name, ParameterType::Point, value);
}

inline void IAppAE::setParameter(const std::string &name, cinder::Vec3f value)
{
	setParameter(name, ParameterType::Point3D, value);
}

inline void IAppAE::setParameter(const std::string &name, cinder::Color value)
{
	setParameter(name, ParameterType::Color, value);
}

}