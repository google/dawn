struct main_inputs {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};


Texture2D tex : register(t0);
RWByteAddressBuffer result : register(u1);
void main_inner(uint3 GlobalInvocationId) {
  int v = int(GlobalInvocationId.x);
  int2 v_1 = int2(int2(v, int(GlobalInvocationId.y)));
  result.Store((0u + (((GlobalInvocationId.y * 128u) + GlobalInvocationId.x) * 4u)), asuint(tex.Load(int3(v_1, int(int(0)))).x));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationId);
}

