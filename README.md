# Cinder-AfterEffects

Create a [cinder](http://libcinder.org/) application which you can control from Adobe After Effects panel. See also [AfterEffects-Cinder(panel)](https://github.com/atarabi/AfterEffects-Cinder) and [a movie](https://vimeo.com/128144102).

## Usage

Include a header file.

```
#include "CinderAfterEffects.h"
```

Create your own app class which inherits from `atarabi::AppAE` instead of `cinder::app::AppNative` and override virtual functions if necessary.

```
class YourApp : public atarabi::AppAE {
public:
	void initializeAE() override; //initializeAE() is called only once after an application has been initialized.
	void setupAE() override; //setupAE() is called when rendering starts.
	void updateAE() override; //Same as update()
	void drawAE() override; //Same as draw()
};
```

If you want to control parameters from AE, call `addParameter` functions in `initializeAE()`.

```
void YourApp::initializeAE()
{
	addParameter("Checkbox", false);
	addParameter("Slider", 0.f);
	addParameter("Point", cinder::Vec2f(0.5f, 0.5f));
	addParameter("Point3D", cinder::Vec3f(0.5f, 0.5f, 0.f));
	addParameter("Color", cinder::Color(1.f, 0.f, 0.f));
}
```

You can get the values of the added parameters by calling `getParameter`.

```
void YourApp::updateAE()
{
	bool checkbox_value = getParameter("Checkbox");
	float slider_value = getParameter("Slider");
	cinder::Vec2f point_value = getParameter("Point");
	cinder::Vec3f point3d_value = getParameter("Point3D");
	cinder::Color color_value = getParameter("Color");
}
```

When your app inherits from `atarabi::AppAEdev` instead, you can control parameters in your app without running AE. It is useful for development.

```
class YourAppDev : public atarabi::AppAEdev {
...
};
```

### Differences between AppNative and AppAE 

|AppNative|AppAE|
|:-:|:-:|
|-|initializeAE|
|setup|setupAE|
|update|updateAE|
|draw|drawAE|
|mouseDown|mouseDownAE|
|mouseUp|mouseUpAE|
|mouseWheel|mouseWheelAE|
|mouseMove|mouseMoveAE|
|mouseDrag|mouseDragAE|
|getElapsedFrames|getCurrentFrame|
|getElapsedSeconds|getCurrentTime|
|timeline|timelineAE|

## Dependencies

Cinder-OSC

## Compatibility

Cinder 0.8.6

## License

MIT