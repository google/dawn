struct main_inputs {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};


Texture2D tex : register(t0);
RWByteAddressBuffer result : register(u1);
void main_inner(uint3 GlobalInvocationId) {
  uint v = 0u;
  result.GetDimensions(v);
  uint v_1 = (min(((GlobalInvocationId.y * 128u) + GlobalInvocationId.x), ((v / 4u) - 1u)) * 4u);
  int v_2 = int(GlobalInvocationId.x);
  int2 v_3 = int2(v_2, int(GlobalInvocationId.y));
  uint3 v_4 = (0u).xxx;
  tex.GetDimensions(0u, v_4.x, v_4.y, v_4.z);
  uint v_5 = min(uint(int(0)), (v_4.z - 1u));
  uint3 v_6 = (0u).xxx;
  tex.GetDimensions(uint(v_5), v_6.x, v_6.y, v_6.z);
  uint2 v_7 = (v_6.xy - (1u).xx);
  int2 v_8 = int2(min(uint2(v_3), v_7));
  result.Store((0u + v_1), asuint(tex.Load(int3(v_8, int(v_5))).x));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationId);
}

