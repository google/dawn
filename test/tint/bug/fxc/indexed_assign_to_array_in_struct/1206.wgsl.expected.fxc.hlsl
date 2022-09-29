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

typedef float3 tint_symbol_3_ret[8];
tint_symbol_3_ret tint_symbol_3(ByteAddressBuffer buffer, uint offset) {
  float3 arr[8] = (float3[8])0;
  {
    for(uint i_1 = 0u; (i_1 < 8u); i_1 = (i_1 + 1u)) {
      arr[i_1] = asfloat(buffer.Load3((offset + (i_1 * 16u))));
    }
  }
  return arr;
}

Particle tint_symbol_2(ByteAddressBuffer buffer, uint offset) {
  const Particle tint_symbol_8 = {tint_symbol_3(buffer, (offset + 0u)), asfloat(buffer.Load((offset + 128u))), asfloat(buffer.Load4((offset + 144u))), asfloat(buffer.Load3((offset + 160u)))};
  return tint_symbol_8;
}

[numthreads(1, 1, 1)]
void main() {
  Particle particle = tint_symbol_2(particles, 0u);
  {
    float3 tint_symbol_1[8] = particle.position;
    tint_symbol_1[sim[0].x] = particle.position[sim[0].x];
    particle.position = tint_symbol_1;
  }
  return;
}
