#version 310 es

struct Simulation {
  uint i;
};

struct Particle {
  vec3 position[8];
  float lifetime;
  vec4 color;
  vec3 velocity;
};

layout(binding = 3, std430) buffer Particles_1 {
  Particle p[];
} particles;
layout(binding = 4) uniform Simulation_1 {
  uint i;
} sim;

void tint_symbol() {
  Particle particle = particles.p[0];
  particle.position[sim.i] = particle.position[sim.i];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
