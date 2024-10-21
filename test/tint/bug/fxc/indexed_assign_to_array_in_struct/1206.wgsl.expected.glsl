#version 310 es


struct Particle {
  vec3 position[8];
  float lifetime;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec4 color;
  vec3 velocity;
  uint tint_pad_3;
};

struct Simulation {
  uint i;
};

layout(binding = 3, std430)
buffer Particles_1_ssbo {
  Particle p[];
} particles;
layout(binding = 4, std140)
uniform sim_block_1_ubo {
  Simulation inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Particle particle = particles.p[0];
  uint v_1 = v.inner.i;
  uint v_2 = v.inner.i;
  particle.position[v_1] = particle.position[v_2];
}
