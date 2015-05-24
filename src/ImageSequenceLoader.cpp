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

#include "ImageSequenceLoader.h"

// avoid struct nil error
#if defined(__APPLE__)
	#undef nil
	#undef Nil
#endif
#include <boost/xpressive/xpressive.hpp>

namespace atarabi {

ImageSequenceLoader::ImageSequenceLoader(const std::string &firstImagePath)
{
	load(firstImagePath);
}

void ImageSequenceLoader::load(const std::string &firstImagePath)
{
	static boost::xpressive::sregex regex = boost::xpressive::sregex::compile("(\\d+)\\.(png|tiff|tif|jpg|jpeg|bmp)$", boost::xpressive::regex_constants::icase);

	reset();
	mFirstImagePath = firstImagePath;

	if (!cinder::fs::exists(mFirstImagePath) || !cinder::fs::is_regular_file(mFirstImagePath))
	{
		return;
	}

	std::string firstImageName = mFirstImagePath.filename().string();

	boost::xpressive::smatch match;
	if (boost::xpressive::regex_search(firstImageName, match, regex))
	{
		std::string prefix = firstImageName.substr(0, firstImageName.size() - match[0].str().size());
		std::string suffix = match[2].str();

		boost::xpressive::sregex sequenceRegex = boost::xpressive::sregex::compile("^" + prefix + "\\d+\\." + suffix + "$");

		mParentPath = mFirstImagePath.parent_path();
		for (cinder::fs::directory_iterator it{ mParentPath }, last{}; it != last; ++it)
		{
			const auto &entry = *it;
			const auto &file = entry.path();
			if (cinder::fs::is_regular_file(file))
			{
				std::string filename = file.filename().string();
				if (boost::xpressive::regex_match(filename, match, sequenceRegex))
				{
					mFileNames.push_back(filename);
				}
			}
		}
	}
}

void ImageSequenceLoader::reset()
{
	mFileNames.clear();
	mFileNames.shrink_to_fit();
}

cinder::ImageSourceRef ImageSequenceLoader::getImage(int32_t frame) const
{
	assert(!mFileNames.empty());

	int32_t numFrames = getNumFrames();

	if (mLoop)
	{
		frame %= numFrames;

		if (frame < 0)
		{
			frame += numFrames;
		}
	}
	else
	{
		if (frame < 0)
		{
			frame = 0;
		}
		else if (frame >= numFrames)
		{
			frame = numFrames - 1;
		}
	}
	
	const std::string &filename = mFileNames[frame];

	return cinder::loadImage(mParentPath / filename);
}

}