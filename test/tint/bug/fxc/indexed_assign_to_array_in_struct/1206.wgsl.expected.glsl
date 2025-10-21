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

layout(binding = 0, std430)
buffer Particles_1_ssbo {
  Particle p[];
} particles;
layout(binding = 1, std140)
uniform sim_block_1_ubo {
  uvec4 inner[1];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (uint(particles.p.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  Particle particle = particles.p[v_2];
  uvec4 v_3 = v.inner[0u];
  uvec4 v_4 = v.inner[0u];
  particle.position[min(v_3.x, 7u)] = particle.position[min(v_4.x, 7u)];
}
