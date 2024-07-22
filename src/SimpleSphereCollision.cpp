#include "dwgSimpleGraphics.h"

// entry point for the app (using WinMain, so no console appears, just the rendering window)
int SimpleSphereCollision()
{
	// init window and rendering with given width, height, and title of the window
	if (!dwgInitApp(1600, 900, "DwG - Game Math"))
		return 1;


	Vector3 c1 = { 0.f, 0.f, 0.f }; // center of sphere 1
	Vector3 c2 = { 1.f, -1.f, 0.f }; // center of sphere 2
	float r = 1.f;

	Vector3 v = { -1.f, 0.f, 0.f };
	v *= 5.f;

	// main game loop, each iteration is a single frame
	while (!dwgShouldClose())
	{
		const double globalTime = dwgGlobalTime();	// global time - time since the start of the app
		const float dt = dwgDeltaTime();			// delta time - time since last frame

		Vector3 a = c2 - c1;
		dwgDebugLine(c1, a, { 1.f, 1.f, 1.f });

		bool overlaping = length(a) < r * 2;

		Vector3 color = overlaping ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.5f, 1.0f, 0.5f);

		c2.setX(sinf(globalTime) * 2);
		c2.setY(sinf(globalTime) * -2);

		dwgDebugSphere(c1, { r,r,r }, color);
		dwgDebugSphere(c2, { r,r,r }, color);

		// prepare camera
		const Point3 eye = { 5.0f, 5.0f, 1.0f };				// eye position
		const Point3 at = { 0.0f, 0.0f, 0.0f };					// what we're looking at
		const Vector3 up = { 0.0f, 0.0f, 1.0f };				// where's the up direction for camera
		const Matrix4 camera = Matrix4::lookAt(eye, at, up);	// create camera view matrix
		const float fov = 120.0f;								// field of view - angle of view of the camera (for perspective)

		// draw scene
		dwgRender(camera, fov);
	}

	// release rendering and window app
	dwgReleaseApp();

	return 0;
}
