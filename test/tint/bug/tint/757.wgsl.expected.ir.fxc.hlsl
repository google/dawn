struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_constants : register(b0) {
  uint4 constants[1];
};
Texture2DArray<float4> myTexture : register(t1);
RWByteAddressBuffer result : register(u3);
void main_inner(uint3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID[2u]) + (2u * GlobalInvocationID[1u])) + GlobalInvocationID[0u]);
  flatIndex = (flatIndex * 1u);
  Texture2DArray<float4> v = myTexture;
  int2 v_1 = int2(int2(GlobalInvocationID.xy));
  int v_2 = int(0);
  float4 texel = float4(v.Load(int4(v_1, v_2, int(0))));
  {
    uint i = 0u;
    while(true) {
      if ((i < 1u)) {
      } else {
        break;
      }
      uint v_3 = (uint((flatIndex + i)) * 4u);
      result.Store((0u + v_3), asuint(texel.x));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

