#include "dwgSimpleGraphics.h"
#include "Exercises.h"

#include <chrono>
#include <thread>

// entry point for the app (using WinMain, so no console appears, just the rendering window)
int ClothSimulation()
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
		Vector3 color = Vector3(-1);
	};

	struct ElasticDistance
	{
		int idx_a = -1;
		int idx_b = -1;

		float distance = 0.7f;

		//float stiffness = 0.1f;
		float compliance = 0.005f;
	};

	const float radius = 0.2f;
	const int numParticles = 12;
	const int numConstrains = numParticles - 1;
	const int numHorizConstrains = numParticles;

	struct Chain
	{
		Particle particles[numParticles];
		ElasticDistance constrains[numConstrains];
		// horizontal constrains
		ElasticDistance horizontalConstrains[numHorizConstrains];

		Vector3 origin;
		Vector3 color;
	};

	const int numChains = 8;

	struct Cloth
	{
		Chain chains[numChains];
		Vector3 origin = Vector3(3.5f, -3.f, 2.5f);
		Vector3 spacing;
	};

	const int numCloths = 3;
	Cloth cloths[numCloths];

	cloths[0].spacing = Vector3(0.f, 0.f, -radius * 2.5f);

	cloths[1].spacing = Vector3(0.f, 0.f, -radius * 2.5f);
	cloths[1].origin.setX(-cloths[1].origin.getX() + (radius * numChains * 2.f));
	cloths[1].origin.setY(-cloths[1].origin.getX());

	cloths[2].origin = Vector3(1.5f, -1.5f, -1.f);
	cloths[2].spacing = Vector3(-radius * 2.5f, -radius * 2.5f, 0.f);

	// chain 0 - red
	// chain 1 - green
	// chain 2 - blue

	// set position and color of chains
	for (Cloth& cl : cloths)
	{
		for (int i = 0; i < numChains; ++i)
		{
			cl.chains[i].origin = cl.origin;
			cl.origin += {-radius * 2.f, radius * 2.f, 0.f};

			cl.chains[i].color = i % 3 == 0 ? Vector3(1.f, 0.3f, 0.5f) : i % 3 == 1 ? Vector3(0.1f, 0.9f, 0.5f) : Vector3(0.3f, 0.3f, 1.f);
		}
	}


	// make particles for each chain and vertical constrains
	for (int i = 0; i < numCloths; ++i)
	{
		Cloth& cl = cloths[i];
		for (Chain& ch : cl.chains)
		{
			for (Particle& p : ch.particles)
			{
				p.pos = ch.origin;
				ch.origin += cl.spacing;
				p.vel = Vector3(0.f);
			}

			ch.particles[0].mass = 0.f;
			if (i > 0)
			{
				ch.particles[numParticles - 1].mass = 0.f;
			}

			for (int j = 0; j < numConstrains; ++j)
			{
				ElasticDistance& c = ch.constrains[j];

				c.idx_a = j;
				c.idx_b = j + 1;

				// this will be the same for every particle because all of the have equal distances between adjacent particles
				c.distance = length(ch.particles[j].pos - ch.particles[j + 1].pos);
			}
		}
	}


	// horizontal constrains
	for (Cloth& cl : cloths)
	{
		for (int i = 0; i < numChains - 1; ++i)
		{
			Chain& ch1 = cl.chains[i];
			Chain& ch2 = cl.chains[i + 1];

			// setup constrains
			for (int j = 0; j < numHorizConstrains; ++j)
			{
				ElasticDistance& c = ch1.horizontalConstrains[j];
				c.idx_a = j;
				c.idx_b = j;

				c.distance = length(ch1.particles[j].pos - ch2.particles[j].pos);
			}
		}
	}


	struct Sphere
	{
		Vector3 pos;
		float radius = 0.5f;
		Vector3 color;
		float mass = 0.f;
	};

	// Setup colliders / spheres
	const int numColiders = 4;

	Sphere colliders[numColiders];

	colliders[0].pos = Vector3(0.f, 0.f, 0.5f);
	colliders[0].color = Vector3(0.5f, 1.0f, 0.5f);

	colliders[1].pos = Vector3(0.f, 2.f, 0.2f);
	colliders[1].color = Vector3(1.f, 0.5f, 0.5f);

	colliders[2].pos = Vector3(-2.5f, -2.f, 3.f);
	colliders[2].color = Vector3(1.f, 0.2f, 0.3f);
	colliders[2].mass = 1.f;

	colliders[3].pos = Vector3(-2.f, -4.f, 3.f);
	colliders[3].color = Vector3(0.5f, 0.2f, 0.8f);
	colliders[3].mass = 1.f;


	// gravitation
	Vector3 acceleration = { 0.f, 0.f, -9.81f };
	//Vector3 acceleration = { 0.f, 0.f, 0.f };

	// 1 iteration = 1 time calculating constrains and collision
	// more iteration = more precise/accurate simulation
	// iteration reduces the stiffness/compliance impact
	// int numIteration = 1;

	const float fixedDeltaTime = 1.f / 60.f;
	float accumulatedTime = 0.f;

	// main game loop, each iteration is a single frame
	while (!dwgShouldClose())
	{
		const double globalTime = dwgGlobalTime();	// global time - time since the start of the app
		const float dt = dwgDeltaTime();			// delta time - time since last frame

		// simulating lag
		//std::chrono::milliseconds timespan(1);
		//std::this_thread::sleep_for(timespan);

		colliders[0].pos.setY(sinf(globalTime) * 2.5f - 1.5f);
		colliders[0].pos.setX(colliders[0].pos.getY() / 2.f + 3.f);

		colliders[1].pos.setY(-sinf(globalTime) * 2.5f + 2.f);
		colliders[1].pos.setX(colliders[1].pos.getY() - 3.f);

		accumulatedTime += dt;

		while (accumulatedTime > fixedDeltaTime)
		{
			accumulatedTime -= fixedDeltaTime;

			for (Cloth& cl : cloths)
			{
				for (Chain& ch : cl.chains)
				{
					// update particles
					for (Particle& p : ch.particles)
					{
						// if the particle has a mass of 0 (should be static) then we don't want to move it
						if (p.mass <= 0.f)
							continue;

						p.prevPos = p.pos;
						p.vel += acceleration * fixedDeltaTime;
						p.pos += p.vel * fixedDeltaTime;
					}

					colliders[2].pos += acceleration * fixedDeltaTime * fixedDeltaTime;
					colliders[3].pos += acceleration * fixedDeltaTime * fixedDeltaTime;
					// resolve constrains
					for (ElasticDistance& c : ch.constrains)
					{
						Particle& p0 = ch.particles[c.idx_a];
						Particle& p1 = ch.particles[c.idx_b];

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
						float displacement = c.distance - distance;

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

						dwgDebugLine(p0.pos, p1.pos, { 1.f, 1.f, 1.f });
					}
				}
			}

			/// HORIZONTAL CONSTRAINS
			for (Cloth& cl : cloths)
			{
				for (int i = 0; i < numChains - 1; ++i)
				{
					Chain& ch1 = cl.chains[i];
					Chain& ch2 = cl.chains[i + 1];


					// resolve constrains
					for (ElasticDistance& c : ch1.horizontalConstrains)
					{
						Particle& p0 = ch1.particles[c.idx_a];
						Particle& p1 = ch2.particles[c.idx_b];

						if (p0.mass != 0.f || p1.mass != 0.f)
						{
							// difference between p1 and p0
							Vector3 diff = p1.pos - p0.pos;

							// distance, length of the difference
							float distance = length(diff);

							// how much we want to move the particle
							float displacement = c.distance - distance;

							// direction in which we will move the particles
							Vector3 dir = normalize(diff);

							float alpha = c.compliance / fixedDeltaTime;

							p0.pos += -dir * displacement * (p0.mass / (p0.mass + p1.mass + alpha)) /** c.stiffness*/;
							p1.pos += dir * displacement * (p1.mass / (p0.mass + p1.mass + alpha)) /** c.stiffness*/;
						}

						dwgDebugLine(p0.pos, p1.pos, { 1.f, 1.f, 1.f });
					}
				}
			}


			// collision and fix velocity
			for (Cloth& cl : cloths)
			{
				for (Chain& ch : cl.chains)
				{
					// resolve collision
					for (Particle& p : ch.particles)
					{
						for (Sphere& col : colliders)
						{
							Vector3 diff = col.pos - p.pos;
							float distance = length(diff);

							// our actual distance between spheres
							float displacment = distance - (col.radius + radius);

							if (displacment < 0.f)
							{
								Vector3 dir = normalize(diff);

								p.pos += dir * displacment * (p.mass / (p.mass + col.mass));
								col.pos += -dir * displacment * (col.mass / (p.mass + col.mass));
							}
						}
					}

					// adjust velocity after resolving constrains and collision 
					for (Particle& p : ch.particles)
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
			}

		}

		for (Cloth& cl : cloths)
		{
			for (Chain& ch : cl.chains)
			{
				// draw particles
				for (Particle& p : ch.particles)
				{
					dwgDebugSphere(p.pos, Vector3(radius), p.color.getX() < 0 ? (p.mass ? Vector3(1.f) : ch.color) : p.color);
				}
			}
		}
		// draw colliders
		for (Sphere& col : colliders)
		{
			dwgDebugSphere(col.pos, Vector3(col.radius), col.color);
		}

		//dwgDebugLine(Vector3(0.f), { 1.f, 0.f, 0.f }, { 1.f, 0.f, 0.f });
		//dwgDebugLine(Vector3(0.f), { 0.f, 1.f, 0.f }, { 0.f, 1.f, 0.f });
		//dwgDebugLine(Vector3(0.f), { 0.f, 0.f, 1.f }, { 0.f, 0.f, 1.f });


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
