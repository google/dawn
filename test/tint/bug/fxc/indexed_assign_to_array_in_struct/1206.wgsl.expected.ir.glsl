#version 310 es


struct Particle {
  vec3 position[8];
  float lifetime;
  vec4 color;
  vec3 velocity;
};

struct Simulation {
  uint i;
};

layout(binding = 3, std430)
buffer Particles_1_ssbo {
  Particle p[];
} particles;
layout(binding = 4, std140)
uniform tint_symbol_2_1_ubo {
  Simulation tint_symbol_1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Particle particle = particles.p[0];
  uint v_1 = v.tint_symbol_1.i;
  uint v_2 = v.tint_symbol_1.i;
  particle.position[v_1] = particle.position[v_2];
}
