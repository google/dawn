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
  int2 v = int2(GlobalInvocationID.xy);
  uint3 v_1 = (0u).xxx;
  myTexture.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = min(uint(int(0)), (v_1.z - 1u));
  uint4 v_3 = (0u).xxxx;
  myTexture.GetDimensions(0u, v_3.x, v_3.y, v_3.z, v_3.w);
  uint v_4 = min(uint(int(0)), (v_3.w - 1u));
  uint4 v_5 = (0u).xxxx;
  myTexture.GetDimensions(uint(v_4), v_5.x, v_5.y, v_5.z, v_5.w);
  uint2 v_6 = (v_5.xy - (1u).xx);
  int2 v_7 = int2(min(uint2(v), v_6));
  int v_8 = int(v_2);
  float4 texel = float4(myTexture.Load(int4(v_7, v_8, int(v_4))));
  {
    uint i = 0u;
    while(true) {
      if ((i < 1u)) {
      } else {
        break;
      }
      uint v_9 = 0u;
      result.GetDimensions(v_9);
      result.Store((0u + (min((flatIndex + i), ((v_9 / 4u) - 1u)) * 4u)), asuint(texel.x));
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

