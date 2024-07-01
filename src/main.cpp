#include "dwgSimpleGraphics.h"

// entry point for the app (using WinMain, so no console appears, just the rendering window)
int WinMain()
{
	// init window and rendering with given width, height, and title of the window
	if (!dwgInitApp(1600, 900, "DwG - Game Math"))
		return 1;


	Vector3 planeta = { 1.f, 1.f, 1.f}; // punkt [m], pozycja startowa planety
	Vector3 sun = { 0.f, 0.f, 0.f };	// punkt [m], pozycja startowa s³oñca

	// ruch balistyczny, ruch po paraboli, jak coœ jest wystrzelone z katapulty to tak leci ³eeee do góry i na dó³ bum 

	Vector3 v = { 0.f, 1.f, 1.f }; // prêdkoœæ planety [m/s]

	//Vector3 acc = { 0.f, 0.f, -9.81f };	// acceleration, przyœpieszenie ziemskie, grawitacja [m/s2]

	// main game loop, each iteration is a single frame
	while (!dwgShouldClose())
	{
		const double globalTime = dwgGlobalTime();	// global time - time since the start of the app
		const float dt = dwgDeltaTime();			// delta time - time since last frame

		// your code goes here...
		
		sun = Vector3(sun.getX(), sun.getY(), sin(globalTime));

		// punkt mo¿na zapisaæ jako wektor od pocz¹tku uk³adu wspó³rzêdnych do tego punktu
		// taki wektor nazywamy wektorem wodz¹cym (wektor po³o¿enia)
		// punkt - punkt = vector pomiêdzy tymi punktami
		Vector3 v_to_sun = sun - planeta;
		Vector3 dir_to_sun = normalize(v_to_sun);

		Vector3 acc = dir_to_sun * 0.5;
		v += acc * dt;

		// [m/s] += [m/s2]
		// Musimy znowu mieæ te same jednostki, czyli musimy pomno¿yæ przez s
		v += acc * dt;


		// [m] += [m/s]
		// Dodawanie [m/s] do [m] jest bez sensu, chcemy dodawaæ te same jednstki czyli metry [m]
		// to, aby dostaæ metry po drugiej stronie musimy pomno¿yæ przez czas tz. [s]
		// czyli nasz¹ delte
		planeta += v * dt;
		// teraz nasza planeta porusza sie 1 metr na sekunde

		// X axis
		dwgDebugLine({ 0.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, { 1.f, 0.f, 0.f });
		// Y axis
		dwgDebugLine({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 1.f, 0.f });
		// Z axis
		dwgDebugLine({ 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 0.f, 1.f });

		dwgDebugSphere(planeta, { 0.4f, 0.4f, 0.4f }, { 0.f, 1.f, 0.f });
		dwgDebugSphere(sun, { 0.4f, 0.4f, 0.4f }, { 1.f, 1.f, 0.f });

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
