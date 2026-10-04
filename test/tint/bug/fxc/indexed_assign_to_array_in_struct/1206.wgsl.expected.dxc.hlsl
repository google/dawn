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
typedef float3 ary_ret[8];
ary_ret v(uint offset) {
  float3 a[8] = (float3[8])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 8u)) {
        break;
      }
      a[v_1] = asfloat(particles.Load3((offset + (v_1 * 16u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_2[8] = a;
  return v_2;
}

Particle v_3(uint offset) {
  float3 v_4[8] = v((offset + 0u));
  Particle v_5 = {v_4, asfloat(particles.Load((offset + 128u))), asfloat(particles.Load4((offset + 144u))), asfloat(particles.Load3((offset + 160u)))};
  return v_5;
}

[numthreads(1, 1, 1)]
void main() {
  uint v_6 = 0u;
  particles.GetDimensions(v_6);
  Particle particle = v_3((0u + (min(0u, ((v_6 / 176u) - 1u)) * 176u)));
  uint v_7 = min(sim[0u].x, 7u);
  uint v_8 = min(sim[0u].x, 7u);
  particle.position[v_7] = particle.position[v_8];
}

