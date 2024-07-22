#include "dwgSimpleGraphics.h"

// entry point for the app (using WinMain, so no console appears, just the rendering window)
int SphereIntersection(bool useDotProduct)
{
	// init window and rendering with given width, height, and title of the window
	if (!dwgInitApp(2500, 1300, "DwG - Game Math"))
		return 1;

	// points
	// sphere
	Vector3 c = { 0.f, 0.f, 0.f };

	// line
	Vector3 p = { 2.f, 0.f, 0.f };

	Vector3 v = { -1.f, 0.f, 0.f };
	v *= 5.f;
	float r = 1.f;

	// main game loop, each iteration is a single frame
	while (!dwgShouldClose())
	{
		const double globalTime = dwgGlobalTime();	// global time - time since the start of the app
		const float dt = dwgDeltaTime();			// delta time - time since last frame

		if(useDotProduct)
		{
			// sphere intersection with dot product

			/*
			White (small) sphere - point p, start of every line
			Purple line - vector v, our line that is the intersecting object
			Green Line - vector cp between point p (start of v) and point c (sphere that is intersected)
			Aqua Line - cp projected onto v, dot product of cp and v
			Big sphere - sphere that is intersected
			*/

			v = Vector3(sinf(globalTime), cosf(globalTime), 0.0f);
			v *= 5.f;

			// point P
			dwgDebugSphere(p, { 0.05f, 0.05f, 0.05f }, Vector3(1.0f, 1.0f, 1.0f));
			Vector3 cp = c - p;

			dwgDebugLine(p, p + cp, { 0.5f, 1.0f, 0.5f });

			// cp projected onto V
			Vector3 h = dot(cp, normalize(v)) * normalize(v);
			dwgDebugLine(p, p + h, { 0.0f, 1.0f, 1.f });

			float x = sqrt(length(cp) * length(cp) - length(h) * length(h));

			bool intersecting = x < r && dot(v, cp) > 0;

			// line V
			dwgDebugLine(p, p + v, { 1.0f, 0.0f, 1.0f });

			Vector3 color = intersecting ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
			dwgDebugSphere(c, { r,r,r }, color);

			// point C
			//dwgDebugSphere(c, { 0.05f, 0.05f, 0.05f }, color);
		}
		else
		{
			// sphere intersection with cross product

			v = Vector3(sinf(globalTime), cosf(globalTime), 0.0f);
			v *= 5.f;

			Vector3 cp = c - p;

			// x:
			//	v cross cp = |v| * |cp| * sin(alpha) * N
			//	norm(v) cross cp = 1 * |cp| * sin(alpha) * N
			//	N has length of 1 so if we take length of the left side we can reduce N to 1
			//	|norm(v) cross cp| = |cp| * sin(alph)

			// sin(alpha) = b / c
			// sin(alpha) = x / |cp|
			// sin(alpha) * |cp| = x

			float x = length(cross(normalize(v), cp));

			bool intersecting = x < r && dot(v, cp) > 0;

			dwgDebugLine(p, p + v, { 1.0f, 0.0f, 1.0f });

			Vector3 color = intersecting ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
			dwgDebugSphere(c, { r,r,r }, color);
		}


		// prepare camera
		const Point3 eye = { 4.0f, 4.0f, 1.0f };				// eye position
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
