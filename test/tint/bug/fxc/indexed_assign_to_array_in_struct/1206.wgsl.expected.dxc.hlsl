struct Particle {
  float3 position[8];
  float lifetime;
  float4 color;
  float3 velocity;
};

ByteAddressBuffer particles : register(t3, space1);
cbuffer cbuffer_sim : register(b4, space1) {
  uint4 sim[1];
};

typedef float3 particles_load_1_ret[8];
particles_load_1_ret particles_load_1(uint offset) {
  float3 arr[8] = (float3[8])0;
  {
    for(uint i_1 = 0u; (i_1 < 8u); i_1 = (i_1 + 1u)) {
      arr[i_1] = asfloat(particles.Load3((offset + (i_1 * 16u))));
    }
  }
  return arr;
}

Particle particles_load(uint offset) {
  Particle tint_symbol_2 = {particles_load_1((offset + 0u)), asfloat(particles.Load((offset + 128u))), asfloat(particles.Load4((offset + 144u))), asfloat(particles.Load3((offset + 160u)))};
  return tint_symbol_2;
}

[numthreads(1, 1, 1)]
void main() {
  Particle particle = particles_load(0u);
  {
    float3 tint_symbol_1[8] = particle.position;
    tint_symbol_1[sim[0].x] = particle.position[sim[0].x];
    particle.position = tint_symbol_1;
  }
  return;
}
