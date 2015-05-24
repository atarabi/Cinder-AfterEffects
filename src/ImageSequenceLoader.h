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

#include "cinder/Filesystem.h"
#include "cinder/ImageIo.h"

#include <string>
#include <vector>

namespace atarabi {

class ImageSequenceLoader {
public:
	ImageSequenceLoader() {}
	ImageSequenceLoader(const std::string &firstImagePath);

	void load(const std::string &firstImagePath);
	void reset();

	void setLoop(bool loop) { mLoop = loop; }
	bool isLoop() const { return mLoop; }

	bool empty() const { return mFileNames.empty(); }
	int32_t getNumFrames() const { return static_cast<int32_t>(mFileNames.size()); }
	cinder::ImageSourceRef getImage(int32_t frame) const;

protected:
	bool mLoop = false;
	cinder::fs::path mFirstImagePath;
	cinder::fs::path mParentPath;
	std::vector<std::string> mFileNames;
};

}