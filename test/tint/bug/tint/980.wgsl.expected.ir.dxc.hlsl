struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer io : register(u0);
float3 Bad(uint index, float3 rd) {
  float3 normal = (0.0f).xxx;
  normal[index] = -(float(sign(rd[index])));
  return normalize(normal);
}

void main_inner(uint idx) {
  uint v_1 = io.Load(12u);
  io.Store3(0u, asuint(Bad(v_1, asfloat(io.Load3(0u)))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

