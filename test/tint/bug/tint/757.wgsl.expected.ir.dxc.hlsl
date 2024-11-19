struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_constants : register(b0) {
  uint4 constants[1];
};
Texture2DArray<float4> myTexture : register(t1);
RWByteAddressBuffer result : register(u3);
void main_inner(uint3 GlobalInvocationID) {
  uint flatIndex = (((4u * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  int2 v = int2(int2(GlobalInvocationID.xy));
  int v_1 = int(int(0));
  float4 texel = float4(myTexture.Load(int4(v, v_1, int(int(0)))));
  {
    uint i = 0u;
    while(true) {
      if ((i < 1u)) {
      } else {
        break;
      }
      result.Store((0u + ((flatIndex + i) * 4u)), asuint(texel.x));
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

