SKIP: FAILED

#version 310 es

struct Particle {
  vec3 position[8];
  float lifetime;
  vec4 color;
  vec3 velocity;
};

struct Particles {
  Particle p[];
};

struct Simulation {
  uint i;
};

Particles particles;
uniform Simulation sim;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Particle particle = particles.p[0];
  particle.position[sim.i] = particle.position[sim.i];
}
error: Error parsing GLSL shader:
ERROR: 0:11: '' : array size required 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
