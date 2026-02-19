# Ball Pit

Tried my hand at a very lightweight 2D physics simulation for circular bodies (particles/discs) with gravity-like forces, boundary collisions, and circle–circle collision response. 

## Features

2D particle/disc bodies: position, velocity, acceleration, mass, radius

Time stepping (explicit Euler integration)

Circle–circle collision detection and impulse-style collision response (with restitution)

Boundary collisions inside an axis-aligned rectangular box

Optional global velocity damping (simple “drag”)

## Non-features / out of scope

This is not a full rigid-body engine.

No rotation, angular velocity, torque, or moments of inertia

No joints/constraints (springs, hinges, motors, etc.)

No stacked resting-contact solver (dense piles may jitter or sink)

No continuous collision detection (fast bodies can tunnel at large time steps)

No broadphase acceleration structure (pair checks scale ~O(n²))

## How it works

Each simulation step:

1) Integrates motion using explicit Euler:

    $v\leftarrow v+a\Delta t$,

    $x\leftarrow x+v\Delta t$,

2) Detects overlaps between circles and between circles and the box walls.

3) Resolves penetrations by separating bodies along the collision normal.

4) Updates velocities along the collision normal using a restitution coefficient (elasticity).

Walls are handled using the same circle–circle collision code as particle–particle collisions. Instead of writing a separate wall-collision solver, the simulation a special particle that represents the boundaries. This “particle as wall” trick keeps the collision pipeline uniform (one collision function for everything) and makes the code simpler, but has tradeoffs:

- It is an approximation: the wall is not truly planar; it is the edge of a huge circle.

- With extreme radii/speeds or far from the local contact region, you can see small artifacts compared to a true plane solver.

- It still inherits discrete-time limitations (tunneling if the timestep is too large).

## Installation and usage

Note: Running the program requires a Windows machine for now. 

Clone repo with `git clone https://github.com/terkol/ball-pit`

Run the `build.bat` file in the root folder to create a new executable and run it. Latest executable can be found in `build/`. 

## Configuration / parameters

Currently there is an input section in the code outlined some comments. 

Inputs (defaults): 

- Mersenne Twister seed (42)

- Range of randomly generated particles' locations `random_pos` (-0.9 to 0.9)

- Range of randomly generated particles' velocities `random_vel` (-0.5 to 0.5)

- Range of randomly generated particles' masses `random_mass` (10 to 12)

- Number of particles `np` (150)

- Elasticity of generated particles `p.ela` (0.6)

- Simulation timestep `dt` (1/240) 

- Friction coefficient `friction` (0.1)

- Constant acceleration of the particles `acc_x` and `acc_y` (-9.81 in the y direction)

- Force applied by WASD controls `push_force` (150)

- Dimensions of the box relative to the dimensions of the window `min/max_x/y`(-1 to 1)

- Properties of the wall; mass `w.mass` (1000000), distance from the edge `w.r` (0.05), elasticity `w.ela` (0.6)

## Stability and limitations

Integration uses explicit Euler, which is simple but not energy-stable. Expect drift at large dt.

Because collisions are detected discretely, very fast particles may pass through each other or walls unless dt is small.

With many particles, performance will degrade due to pairwise checks.

Practical guidance:

Use dt ≈ 1/60 or smaller for interactive sims.

Keep maximum speed such that per-step travel is less than the smallest radius (rule of thumb).

## License

### MIT
