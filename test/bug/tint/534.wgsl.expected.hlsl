void Set_uint4(inout uint4 vec, int idx, uint val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
    case 2: vec[2] = val; break;
    case 3: vec[3] = val; break;
  }
}

Texture2D<float4> src : register(t0, space0);
Texture2D<float4> tint_symbol : register(t1, space0);
RWByteAddressBuffer output : register(u2, space0);
cbuffer cbuffer_uniforms : register(b3, space0) {
  uint4 uniforms[1];
};

uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

struct tint_symbol_2 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_2 tint_symbol_1) {
  const uint3 GlobalInvocationID = tint_symbol_1.GlobalInvocationID;
  int2 tint_tmp;
  src.GetDimensions(tint_tmp.x, tint_tmp.y);
  int2 size = tint_tmp;
  int2 dstTexCoord = int2(GlobalInvocationID.xy);
  int2 srcTexCoord = dstTexCoord;
  const uint scalar_offset = (0u) / 4;
  if ((uniforms[scalar_offset / 4][scalar_offset % 4] == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1);
  }
  float4 srcColor = src.Load(int3(srcTexCoord, 0));
  float4 dstColor = tint_symbol.Load(int3(dstTexCoord, 0));
  bool success = true;
  uint4 srcColorBits = uint4(0u, 0u, 0u, 0u);
  uint4 dstColorBits = uint4(dstColor);
  {
    uint i = 0u;
    while (true) {
      const uint scalar_offset_1 = (12u) / 4;
      if (!((i < uniforms[scalar_offset_1 / 4][scalar_offset_1 % 4]))) { break; }
      Set_uint4(srcColorBits, i, ConvertToFp16FloatValue(srcColor[i]));
      bool tint_tmp_1 = success;
      if (tint_tmp_1) {
        tint_tmp_1 = (srcColorBits[i] == dstColorBits[i]);
      }
      success = (tint_tmp_1);
      i = (i + 1u);
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    output.Store((4u * outputIndex), asuint(uint(1)));
  } else {
    output.Store((4u * outputIndex), asuint(uint(0)));
  }
  return;
}
