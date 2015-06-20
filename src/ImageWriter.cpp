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

#include "ImageWriter.h"
#include "cinder/ImageIo.h"
#include "cinder/ip/Flip.h"
#include "cinder/ip/Premultiply.h"

namespace atarabi {

ImageWriter::ImageWriter() : mUnpremultiply{ false }, mAbort{ false }, mImages{ DEFAULT_BUFFER_SIZE }
{
	initThread();
}

ImageWriter::ImageWriter(std::size_t buffer_size) : mUnpremultiply{ false }, mAbort{ false }, mImages{ buffer_size }
{
	initThread();
}

ImageWriter::~ImageWriter()
{
	mAbort = true;
	mImages.cancel();
	mThread->join();
}

void ImageWriter::pushImage(const std::string &path, const cinder::Surface &surface)
{
	mImages.pushFront({ path, surface });
}

bool ImageWriter::empty()
{
	return !mImages.isNotEmpty();
}

void ImageWriter::initThread()
{
	mThread = std::make_shared<std::thread>(std::bind(&ImageWriter::writeImage, this));
}

void ImageWriter::writeImage()
{
	cinder::ThreadSetup threadSetup;

	while (!mAbort)
	{
		Image image;
		mImages.popBack(&image);

		cinder::fs::path path = image.path();
		auto &surface = image.surface();

		if (surface.getDataStore())
		{
			try
			{
				if (mUnpremultiply)
				{
					cinder::ip::unpremultiply(&surface);
				}
				cinder::ip::flipVertical(&surface);
				cinder::writeImage(path, surface);
			}
			//when window is minimized
			catch (...)
			{
				//pass
			}
		}
	}
}

}