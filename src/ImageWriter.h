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

#include "cinder/Surface.h"
#include "cinder/Thread.h"
#include "cinder/ConcurrentCircularBuffer.h"
#include <string>

namespace atarabi {

class ImageWriter {
	class Image {
	public:
		Image() {}
		Image(const std::string &path, const cinder::Surface &surface) : mPath(path), mSurface{ surface }{}

		const std::string &path() const { return mPath; }
		cinder::Surface &surface() { return mSurface; }

	private:
		std::string mPath;
		cinder::Surface mSurface;
	};

public:
	static const int DEFAULT_BUFFER_SIZE = 150;

	ImageWriter();
	ImageWriter(std::size_t buffer_size);

	~ImageWriter();

	void setUnpremultiply(bool unpremultiply) { mUnpremultiply = unpremultiply; }

	void pushImage(const std::string &path, const cinder::Surface &surface);
	bool empty();

private:
	void initThread();
	void writeImage();

	std::shared_ptr<std::thread> mThread;
	cinder::ConcurrentCircularBuffer<Image> mImages;
	bool mUnpremultiply;
	bool mAbort;
};

}