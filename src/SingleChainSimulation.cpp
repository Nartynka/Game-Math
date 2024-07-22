#include "dwgSimpleGraphics.h"

#include <chrono>
#include <thread>

// entry point for the app (using WinMain, so no console appears, just the rendering window)
int SingleChainSimulation()
{
	// init window and rendering with given width, height, and title of the window
	if (!dwgInitApp(1600, 900, "DwG - Game Math"))
		return 1;


	struct Particle
	{
		Vector3 pos;
		Vector3 prevPos;
		Vector3 vel;
		float mass = 1.f;
	};

	struct ElasticDistance
	{
		int idx_a = -1;
		int idx_b = -1;

		float Distance = 1.f;
		float stiffness = 0.1f;
		float compliance = 0.05f;
	};

	const int numParticles = 10;
	Particle particles[numParticles];

	Vector3 origin = { 0.f, 0.f, 1.f };

	float radius = 0.2f;

	for (Particle& p : particles)
	{
		p.pos = origin;
		origin += Vector3(2.f * radius, 0.f, 0.f);
		p.vel = Vector3(0.f);
	}

	particles[0].mass = 0.f;

	const int numConstrains = numParticles - 1;
	ElasticDistance constrains[numConstrains];

	for (int i = 0; i < numConstrains; ++i)
	{
		ElasticDistance& c = constrains[i];

		c.idx_a = i;
		c.idx_b = i + 1;

		c.Distance = length(particles[i].pos - particles[i + 1].pos);
	}

	struct Sphere
	{
		Vector3 pos;
		float radius = 0.5f;
		Vector3 color;
	};

	Sphere collider;
	collider.pos = Vector3(0.f, 2.f, 0.f);
	collider.color = Vector3(0.5f, 1.0f, 0.5f);

	// gravitation
	Vector3 acceleration = { 0.f, 0.f, -9.81f };

	// 1 iteration = 1 time calculating constrains and collision
	// more iteration = more precise/accurate simulation
	// iteration reduces the stiffness/compliance impact
	int numIteration = 1;

	const float fixedDeltaTime = 1.f / 60.f;
	float accumulatedTime = 0.f;

	// main game loop, each iteration is a single frame
	while (!dwgShouldClose())
	{
		const double globalTime = dwgGlobalTime();	// global time - time since the start of the app
		const float dt = dwgDeltaTime();			// delta time - time since last frame

		// simulating lag
		std::chrono::milliseconds timespan(1);
		std::this_thread::sleep_for(timespan);

		collider.pos = { 0.f, sinf(globalTime) * 2.f, 0.f };

		accumulatedTime += dt;

		while (accumulatedTime > fixedDeltaTime)
		{
			accumulatedTime -= fixedDeltaTime;

			// update particles
			for (Particle& p : particles)
			{
				// if the particle has a mass of 0 (should be static) then we don't want to move it
				if (p.mass <= 0.f)
					continue;

				p.prevPos = p.pos;
				p.vel += acceleration * fixedDeltaTime;
				p.pos += p.vel * fixedDeltaTime;
			}

			for (int i = 0; i < numIteration; ++i)
			{
				// resolve constrains
				for (ElasticDistance& c : constrains)
				{
					Particle& p0 = particles[c.idx_a];
					Particle& p1 = particles[c.idx_b];

					// difference between p1 and p0
					Vector3 diff = p1.pos - p0.pos;

					// distance, length of the difference
					float distance = length(diff);

					// how much we want to move the particle
					// c.Distance is some constant distance that we want to preserve
					// distance is the actual distance
					// displacement is a difference between these distances
					// we have to move each particles by the half of it to get the desired distance (c.distance)
					// or the second one by all if the first particle is static / has a mass of 0
					float displacement = c.Distance - distance;

					// direction in which we will move the particles
					Vector3 dir = normalize(diff);

					float alpha = c.compliance / fixedDeltaTime;

					// first (zero) particle we want to move "up" so we have to invert (minus) the direction
					// The second multiplication is to determine if we have to apply all or half of the displacement (it's simple arithmetic average)
					// if p0 has a mass of 0 (should not move) then 0 / 0 + 1 = 0 - we don't add anything to position of p0
					// and in second particle: 1 / 0 + 1 = 1 - we add all the displacement
					// if both particle have a mass of 1 then 1 / 1+1 = 0.5 - we add half of the displacement
					// this multiplication is like an if statement to determine if we want to add half to both particles or all to one particle
					// the stiffness is how many percent of resolving constrain we want e.g. 0.3 is 30% so it will be pretty elastic
					// compliance is the inverse stiffness, the advantage of it is that 0.f means stiff 
					// In stiffness there is not a value that determines 100% stiff, it could be stiffness = 1.f but apparently it doesn't always work?
					p0.pos += -dir * displacement * (p0.mass / (p0.mass + p1.mass + alpha)) /** c.stiffness*/;
					p1.pos += dir * displacement * (p1.mass / (p0.mass + p1.mass + alpha)) /** c.stiffness*/;

					//dwgDebugLine(p0.pos, p1.pos, { 1.f, 1.f, 1.f });
				}

				// resolve collision
				for (Particle& p : particles)
				{
					Vector3 diff = collider.pos - p.pos;
					float distance = length(diff);

					// our actual distance between spheres
					float displacment = distance - (collider.radius + radius);

					//bool overlaping = displacment < 0.f;
					//collider.color = overlaping ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.5f, 1.0f, 0.5f);

					if (displacment < 0.f)
					{
						Vector3 dir = normalize(diff);
						p.pos += dir * displacment;
					}
				}
			}

			// adjust velocity after resolving constrains and collision 
			for (Particle& p : particles)
			{
				// p.prevPos is the position before resolving constrains and collision!
				// and we have to fix the velocity because it is based on position before resolving constrains and collision
				// p.pos - p.prevPos gives us the distance that the particle moved after resolving constrains and collision
				// so it gives us our current velocity that the particle already moved by resoling constrains and collision
				// 
				// when we set the position in next frame we multiplie the velocity times delta time (p.pos += p.vel * fixedDeltaTime;)
				// so here we have to divide our velocity times delta time (to match our units)
				// velocity is [m/s]
				p.vel = (p.pos - p.prevPos) / fixedDeltaTime;
			}
		}



		// draw particles
		for (Particle& p : particles)
		{
			dwgDebugSphere(p.pos, Vector3(radius), { 1.f, 1.f, 1.f });
		}

		// draw collider
		dwgDebugSphere(collider.pos, Vector3(collider.radius), collider.color);

		// prepare camera
		const Point3 eye = { 3.0f, 3.0f, 1.0f };				// eye position
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
