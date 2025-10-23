struct Light {
  float3 position;
  float3 colour;
};


ByteAddressBuffer lights : register(t1);
Light v(uint offset) {
  Light v_1 = {asfloat(lights.Load3((offset + 0u))), asfloat(lights.Load3((offset + 16u)))};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  uint v_2 = 0u;
  lights.GetDimensions(v_2);
  uint v_3 = ((v_2 / 32u) - 1u);
  v((0u + (min(uint(int(0)), v_3) * 32u)));
}

