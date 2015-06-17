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

#include "cinder/Camera.h"
#include "cinder/Matrix44.h"

namespace atarabi {

class CameraAE : public cinder::CameraPersp {
public:
	struct Parameter {
		float fov;
		cinder::mat4 cameraMatrix;
	};

	void setParameter(const Parameter &parameter)
	{
		setFov(parameter.fov);

		auto &cameraMatrix = parameter.cameraMatrix;

		mInverseModelViewMatrix = cameraMatrix;
		mInverseModelViewMatrix = cinder::scale(cameraMatrix, cinder::vec3{ 1.f, -1.f, -1.f });
		mInverseModelViewCached = true;

		mViewMatrix = cinder::inverse(mInverseModelViewMatrix);
		mModelViewCached = true;
	}
};

}