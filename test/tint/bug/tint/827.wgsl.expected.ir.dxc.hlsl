struct main_inputs {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};


Texture2D tex : register(t0);
RWByteAddressBuffer result : register(u1);
void main_inner(uint3 GlobalInvocationId) {
  uint v = (uint(((GlobalInvocationId[1u] * 128u) + GlobalInvocationId[0u])) * 4u);
  int v_1 = int(GlobalInvocationId[0u]);
  int2 v_2 = int2(int2(v_1, int(GlobalInvocationId[1u])));
  result.Store((0u + v), asuint(tex.Load(int3(v_2, int(int(0)))).x));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationId);
}

