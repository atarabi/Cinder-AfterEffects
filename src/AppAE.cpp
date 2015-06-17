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

#include "AppAE.h"
#include "cinder/Utilities.h"
#include "cinder/CinderMath.h"
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <cassert>
#include <stdexcept>
#include <chrono>
#include <thread>

namespace atarabi {

namespace {

template<class Integer>
std::string zfill(Integer number, int width)
{
	std::stringstream ss;
	ss << std::setw(width) << std::setfill('0') << number;
	return ss.str();
}

std::string parmeterTypeToString(ParameterType type)
{
	switch (type)
	{
		case ParameterType::Checkbox:
			return "checkbox";
		case ParameterType::Slider:
			return "slider";
		case ParameterType::Point:
			return "point";
		case ParameterType::Point3D:
			return "point3d";
		case ParameterType::Color:
			return "color";
	}

	assert(0);
	throw std::invalid_argument("invalid parameter type");
}

void addValueToMessage(cinder::osc::Message &message, ParameterType type, ParameterValue value)
{
	switch (type)
	{
		case ParameterType::Checkbox:
			message.addIntArg(static_cast<int32_t>(value.checkbox.value));
			break;
		case ParameterType::Slider:
			message.addFloatArg(value.slider.value);
			break;
		case ParameterType::Point:
			message.addFloatArg(value.point.x);
			message.addFloatArg(value.point.y);
			break;
		case ParameterType::Point3D:
			message.addFloatArg(value.point3d.x);
			message.addFloatArg(value.point3d.y);
			message.addFloatArg(value.point3d.z);
			break;
		case ParameterType::Color:
			message.addFloatArg(value.color.r);
			message.addFloatArg(value.color.g);
			message.addFloatArg(value.color.b);
			break;
	}
}

} //anonymous namespace


void AppAE::setup()
{
	mPath = cinder::getHomeDirectory().string();
	mSender.setup("localhost", CLIENT_PORT);
	mListener.setup(SERVER_PORT);

	initializeAE();
	transition(State::Setup);
}

void AppAE::update()
{
	if (mState == State::Render)
	{
		timelineAE().stepTo(getCurrentTime());

		if (useFbo())
		{
			cinder::gl::ScopedFramebuffer scopedFrameBuffer{ mFbo };
			updateAE();
		}
		else
		{
			cinder::gl::ScopedFramebuffer scopedFrameBuffer{ GL_FRAMEBUFFER, 0 };
			updateAE();
		}
	}
}

void AppAE::draw()
{
	if (mState == State::Render)
	{
		//draw
		{
			if (useFbo())
			{
				cinder::gl::ScopedFramebuffer scopedFrameBuffer{ mFbo };
				drawAE();
			}
			else
			{
				cinder::gl::ScopedFramebuffer scopedFrameBuffer{ GL_FRAMEBUFFER, 0 };
				drawAE();
			}
		}

		//write
		if (mWrite)
		{
			writeImage();
		}

		//reply
		{
			cinder::osc::Message reply;
			reply.setAddress("/cinder/render/" + std::to_string(mCurrentFrame));
			mSender.sendMessage(reply);
		}

		++mCurrentFrame;

		if (mCurrentFrame >= mDuration)
		{
			transition(State::Setdown);
		}
	}

	while (mListener.hasWaitingMessages())
	{
		processMessage();
	}
}

void AppAE::mouseDown(cinder::app::MouseEvent event)
{
	if (mState == State::Render)
	{
		mouseDownAE(event);
	}
}

void AppAE::mouseUp(cinder::app::MouseEvent event)
{
	if (mState == State::Render)
	{
		mouseUpAE(event);
	}
}

void AppAE::mouseWheel(cinder::app::MouseEvent event)
{
	if (mState == State::Render)
	{
		mouseWheelAE(event);
	}
}

void AppAE::mouseMove(cinder::app::MouseEvent event)
{
	if (mState == State::Render)
	{
		mouseMoveAE(event);
	}
}

void AppAE::mouseDrag(cinder::app::MouseEvent event)
{
	if (mState == State::Render)
	{
		mouseDragAE(event);
	}
}

void AppAE::addParameter(const std::string &name, ParameterType type, ParameterValue initialValue)
{
	assert(mState == State::Uninitialized && name != "CameraAE");

	if (mGetters.count(name) == 0)
	{
		mGetters.insert(std::make_pair(name, Getter{ static_cast<uint32_t>(mGetters.size()), name, type, initialValue }));
	}
}

ParameterValue AppAE::getParameter(const std::string &name, uint32_t frame) const
{
	assert(mState == State::Render && mGetters.count(name) > 0);

	if (frame >= mDuration)
	{
		frame = mDuration - 1;
	}

	const auto it = mGetters.find(name);
	const auto &getter = it->second;

	return getter.values[frame];
}

CameraAE::Parameter AppAE::getCameraParameter(uint32_t frame) const
{
	assert(mState == State::Render && mUseCamera);

	if (frame >= mDuration)
	{
		frame = mDuration - 1;
	}

	return mCameraGetters[frame];
}

void AppAE::setParameter(const std::string &name, ParameterType type, ParameterValue value, uint32_t frame)
{
	assert(mState == State::Render);

	if (mSetters.count(name) == 0)
	{
		mSetters.insert(std::make_pair(name, Setter{ static_cast<uint32_t>(mSetters.size()), name, type }));
	}

	mSetters[name].values.push_back(std::make_pair(frame, value));
}

void AppAE::setCameraParameter(const cinder::Camera &camera)
{
	assert(mState == State::Render);

	mCameraSetters.push_back(std::make_pair(mCurrentFrame, CameraAE::Parameter{ camera.getFov(), camera.getInverseViewMatrix() }));
}

bool AppAE::useFbo() const
{
	return mUseFbo && mWrite;
}

bool AppAE::isParameterCached() const
{
	if (mUseCamera)
	{
		if (mCameraGetters.size() != mDuration)
		{
			return false;
		}
	}

	for (const auto& getter : mGetters)
	{
		const auto &values = getter.second.values;
		if (values.size() != mDuration)
		{
			return false;
		}
	}

	return true;
}

void AppAE::transition(State state)
{
	if (mState == state)
	{
		return;
	}

	mState = state;

	switch (state)
	{
		case State::Setup:
			mCameraSetters.clear();
			mSetters.clear();
			break;
		case State::Render:
			getWindow()->setAlwaysOnTop(true);

			if (useFbo())
			{
				setWindowSize({ 640, 360 });
				mFbo = cinder::gl::Fbo::create(mWidth, mHeight, cinder::gl::Fbo::Format{}.samples(16));
				auto area = mFbo->getBounds();
				cinder::gl::viewport(std::make_pair(cinder::ivec2{ 0, 0 }, area.getSize()));
				cinder::gl::setMatricesWindow(mFbo->getSize());
			}
			else
			{
				setWindowSize({ mWidth, mHeight });
			}

			mCurrentFrame = 0;
			timelineAE().clear();
			timelineAE().stepTo(0.f);
			setupAE();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			break;
		case State::Setdown:
			getWindow()->setAlwaysOnTop(false);
			setdown();
			break;
	}
}

void AppAE::setdown()
{
	//wait for writing images to end
	while (!mWriter.empty())
	{
		cinder::osc::Message reply;
		reply.setAddress("/cinder/render/write");
		mSender.sendMessage(reply);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	//setdown
	static const int MAX_CAMERA_ARG_NUM = 30;
	static const int MAX_ARG_NUM = 150;

	if (mWrite)
	{
		if (!mCameraSetters.empty())
		{
			std::string prefix = "/cinder/setdown/cameraAE/";
			int valueSize = static_cast<int>(mCameraSetters.size());

			//begin
			{
				cinder::osc::Message reply;
				reply.setAddress(prefix + "begin");
				reply.addStringArg("camera");
				reply.addIntArg(static_cast<int32_t>(static_cast<double>(valueSize) / MAX_CAMERA_ARG_NUM + 0.5f));
				mSender.sendMessage(reply);
			}

			for (int i = 0, n = 0; i < valueSize; i += MAX_CAMERA_ARG_NUM, ++n)
			{
				cinder::osc::Message reply;

				reply.setAddress(prefix + std::to_string(n));

				for (int j = 0, total = std::min(MAX_CAMERA_ARG_NUM, valueSize - i); j < total; ++j)
				{
					auto &pair = mCameraSetters[i + j];
					auto frame = pair.first;
					auto &value = pair.second;
					float fov = cinder::toRadians(value.fov);
					auto cameraMatrix = value.cameraMatrix;
					cameraMatrix = cinder::scale(cameraMatrix, cinder::vec3{ 1.f, -1.f, -1.f });

					//calc position
					cinder::vec3 position{ cameraMatrix[0][3], cameraMatrix[1][3], cameraMatrix[2][3] };

					//calc orientation
					cinder::vec3 orientation{};
					{
						orientation.y = std::asin(cameraMatrix[0][2]);

						float cosY = std::cos(orientation.y);
						if (cosY != 0.f)
						{
							orientation.x = std::atan2(-cameraMatrix[1][2], cameraMatrix[2][2]);
							orientation.z = std::asin(-cameraMatrix[0][1] / cosY);
							if (cameraMatrix[0][0] < 0.f)
							{
								orientation.z = static_cast<float>(M_PI)-orientation.z;
							}
						}
						else
						{
							orientation.x = std::atan2(cameraMatrix[2][1], cameraMatrix[1][1]);
							orientation.y = 0.5f * static_cast<float>(M_PI);
							orientation.z = 0.f;
						}

						orientation *= 57.295779513082321f; // ( x * 180 / PI )
					}

					//calc zoom
					float zoom = getHeight() / (2.f * std::tan(fov / 2.f));

					//add args
					reply.addIntArg(frame);
					reply.addFloatArg(position.x);
					reply.addFloatArg(position.y);
					reply.addFloatArg(position.z);
					reply.addFloatArg(orientation.x);
					reply.addFloatArg(orientation.y);
					reply.addFloatArg(orientation.z);
					reply.addFloatArg(zoom);
				}

				mSender.sendMessage(reply);
			}
		}

		std::vector<Setter*> setters;
		for (auto &pair : mSetters)
		{
			setters.push_back(&pair.second);
		}

		std::sort(setters.begin(), setters.end(), [](const Setter *lhs, const Setter *rhs) -> bool {
			return lhs->id < rhs->id;
		});

		for (auto &setter : setters)
		{
			auto &name = setter->name;
			ParameterType type = setter->type;
			auto &values = setter->values;
			std::sort(values.begin(), values.end(), [](const Setter::Value &lhs, const Setter::Value &rhs) -> bool {
				return lhs.first < rhs.first;
			});
			int valueSize = static_cast<int>(values.size());
			std::string prefix = "/cinder/setdown/" + name + "/";

			//begin
			{
				cinder::osc::Message reply;
				reply.setAddress(prefix + "begin");
				reply.addStringArg(parmeterTypeToString(type));
				reply.addIntArg(static_cast<int32_t>(static_cast<double>(valueSize) / MAX_ARG_NUM + 0.5f));
				mSender.sendMessage(reply);
			}

			for (int i = 0, n = 0; i < valueSize; i += MAX_ARG_NUM, ++n)
			{
				cinder::osc::Message reply;

				reply.setAddress(prefix + std::to_string(n));

				for (int j = 0, total = std::min(MAX_ARG_NUM, valueSize - i); j < total; ++j)
				{
					auto &pair = values[i + j];
					auto frame = pair.first;
					auto &value = pair.second;
					reply.addIntArg(frame);
					addValueToMessage(reply, type, value);
				}

				mSender.sendMessage(reply);
			}
		}
	}

	//renderend
	{
		cinder::osc::Message reply;
		reply.setAddress("/cinder/renderend");

		//path
		std::string sequencePath = mPath + "/" + mFileName + "_" + zfill(0, 5) + ".png";
		reply.addStringArg(sequencePath);

		//executable path
		auto &argv = getCommandLineArgs();
		cinder::fs::path executablePath = cinder::fs::system_complete(argv[0]);
		reply.addStringArg(executablePath.string());

		//source path
		reply.addStringArg(mSourcePath);

		//source time
		reply.addFloatArg(mSourceTime);

		mSender.sendMessage(reply);
	}

	transition(State::Setup);
}

void AppAE::processMessage()
{
	cinder::osc::Message message;
	mListener.getNextMessage(&message);

	auto paths = cinder::split(message.getAddress(), '/');

	if (!paths.empty())
	{
		paths.erase(paths.begin());
	}

	if (paths.size() < 2 || paths[0] != "cinder")
	{
		return;
	}

	if (paths[1] == "setup")
	{
		processSetupMessage(message, paths);
	}
	else if (paths[1] == "prerender")
	{
		processPrerenderMessage(message, paths);
	}
	else if (paths[1] == "render")
	{
		transition(State::Render);
	}
	else if (paths[1] == "quit")
	{
		transition(State::Setdown);
	}
}

void AppAE::processSetupMessage(const cinder::osc::Message &message, const std::vector<std::string> &paths)
{
	std::string path = message.getArgAsString(SETUP_ARG_PATH);
	if (!path.empty())
	{
		mPath = path;
	}

	std::string fileName = message.getArgAsString(SETUP_ARG_FILENAME);
	if (!fileName.empty())
	{
		mFileName = fileName;
	}

	int32_t cache = message.getArgAsInt32(SETUP_ARG_CACHE);
	mCache = cache ? true : false;

	int32_t write = message.getArgAsInt32(SETUP_ARG_WRITE);
	mWrite = write ? true : false;

	int32_t fbo = message.getArgAsInt32(SETUP_ARG_FBO);
	mUseFbo = fbo ? true : false;

	float fps = message.getArgAsFloat(SETUP_ARG_FPS);
	if (fps > 0.f)
	{
		mFps = fps;
		setFrameRate(fps);
	}

	int32_t duration = message.getArgAsInt32(SETUP_ARG_DURATION);
	if (duration > 0)
	{
		mDuration = duration;
	}

	int32_t width = message.getArgAsInt32(SETUP_ARG_WIDTH);
	int32_t height = message.getArgAsInt32(SETUP_ARG_HEIGHT);
	if (width > 0 && height > 0)
	{
		mWidth = width;
		mHeight = height;
	}

	std::string sourcePath = message.getArgAsString(SETUP_ARG_SOURCE);
	if (!sourcePath.empty())
	{
		float sourceTime = message.getArgAsFloat(SETUP_ARG_SOURCETIME);
		mSourcePath = sourcePath;
		mSourceTime = sourceTime;
	}

	//reply
	cinder::osc::Message reply;
	reply.setAddress(message.getAddress());

	//camera
	reply.addIntArg(mUseCamera);
	//cache
	reply.addIntArg(isParameterCached());

	std::vector<Getter*> getters;
	for (auto &pair : mGetters)
	{
		getters.push_back(&pair.second);
	}

	std::sort(getters.begin(), getters.end(), [](const Getter *lhs, const Getter *rhs) -> bool {
		return lhs->id < rhs->id;
	});

	for (auto &getter : getters)
	{
		reply.addStringArg(getter->name);
		reply.addStringArg(parmeterTypeToString(getter->type));
		addValueToMessage(reply, getter->type, getter->initialValue);
	}

	mSender.sendMessage(reply);
}

void AppAE::processPrerenderMessage(const cinder::osc::Message &message, const std::vector<std::string> &paths)
{
	std::string err = "";
	do {
		if (paths.size() < 4)
		{
			err = "invalid address";
			break;
		}
		const std::string &parameter_name = paths[2];
		const std::string &times = paths[3];

		if (mGetters.count(parameter_name) == 0 && !(mUseCamera && parameter_name == "CameraAE"))
		{
			err = "cannot find a parameter name";
			break;
		}

		int argNum = message.getNumArgs();


		if (parameter_name == "CameraAE")
		{
			auto &camera_getters = mCameraGetters;

			if (times == "begin")
			{
				camera_getters.clear();
			}

			for (int i = 0; i < argNum; i += 13)
			{
				float fov = message.getArgAsFloat(i);
				cinder::mat4 matrix{
					message.getArgAsFloat(i + 1), message.getArgAsFloat(i + 2), message.getArgAsFloat(i + 3), 0.f,
					message.getArgAsFloat(i + 4), message.getArgAsFloat(i + 5), message.getArgAsFloat(i + 6), 0.f,
					message.getArgAsFloat(i + 7), message.getArgAsFloat(i + 8), message.getArgAsFloat(i + 9), 0.f,
					message.getArgAsFloat(i + 10), message.getArgAsFloat(i + 11), message.getArgAsFloat(i + 12), 1.f
				};

				camera_getters.push_back({ fov, matrix });
			}

			if (times == "last")
			{
				if (camera_getters.size() != mDuration)
				{
					err = "invalid arg size";
					camera_getters.clear();
					break;
				}
			}
		}
		else
		{
			auto &parameter = mGetters[parameter_name];
			auto type = parameter.type;
			auto &values = parameter.values;

			if (times == "begin")
			{
				values.clear();
			}

			switch (type) {
				case ParameterType::Checkbox:
					for (int i = 0; i < argNum; ++i)
					{
						ParameterValue value;
						value.checkbox.value = message.getArgAsInt32(i) ? true : false;
						values.push_back(value);
					}
					break;
				case ParameterType::Slider:
					for (int i = 0; i < argNum; ++i)
					{
						ParameterValue value;
						value.slider.value = message.getArgAsFloat(i);
						values.push_back(value);
					}
					break;
				case ParameterType::Point:
					for (int i = 0; i < argNum; i += 2)
					{
						ParameterValue value;
						value.point.x = message.getArgAsFloat(i);
						value.point.y = message.getArgAsFloat(i + 1);
						values.push_back(value);
					}
					break;
				case ParameterType::Point3D:
					for (int i = 0; i < argNum; i += 3)
					{
						ParameterValue value;
						value.point3d.x = message.getArgAsFloat(i);
						value.point3d.y = message.getArgAsFloat(i + 1);
						value.point3d.z = message.getArgAsFloat(i + 2);
						values.push_back(value);
					}
					break;
				case ParameterType::Color:
					for (int i = 0; i < argNum; i += 3)
					{
						ParameterValue value;
						value.color.r = message.getArgAsFloat(i);
						value.color.g = message.getArgAsFloat(i + 1);
						value.color.b = message.getArgAsFloat(i + 2);
						values.push_back(value);
					}
					break;
			}

			if (times == "last")
			{
				if (values.size() != mDuration)
				{
					err = "invalid arg size";
					values.clear();
					break;
				}
			}
		}

	} while (0);

	cinder::osc::Message reply;
	reply.setAddress(message.getAddress());
	reply.addStringArg(err);
	mSender.sendMessage(reply);
}

void AppAE::writeImage()
{
	std::string path = mPath + "/" + mFileName + "_" + zfill(mCurrentFrame, 5) + ".png";

	if (useFbo())
	{
		cinder::Surface surface = mFbo->readPixels8u(mFbo->getBounds());

		mWriter.pushImage(path, surface);
	}
	else
	{
		cinder::gl::ScopedFramebuffer scopedFrameBuffer{ GL_FRAMEBUFFER, 0 };

		int width = getWindowWidth();
		int height = getWindowHeight();

		cinder::Surface surface{ width, height, true };
		glFlush();
		GLint oldPackAlignment;
		glGetIntegerv(GL_PACK_ALIGNMENT, &oldPackAlignment);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, surface.getData());
		glPixelStorei(GL_PACK_ALIGNMENT, oldPackAlignment);

		mWriter.pushImage(path, surface);
	}


}

} //namespace atarabi