Texture2D<float4> src : register(t0, space0);
Texture2D<float4> tint_symbol : register(t1, space0);
RWByteAddressBuffer output : register(u2, space0);
cbuffer cbuffer_uniforms : register(b3, space0) {
  uint4 uniforms[2];
};

bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.001f);
}

struct tint_symbol_2 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_2 tint_symbol_1) {
  const uint3 GlobalInvocationID = tint_symbol_1.GlobalInvocationID;
  int2 tint_tmp;
  src.GetDimensions(tint_tmp.x, tint_tmp.y);
  const int2 srcSize = tint_tmp;
  int2 tint_tmp_1;
  tint_symbol.GetDimensions(tint_tmp_1.x, tint_tmp_1.y);
  const int2 dstSize = tint_tmp_1;
  const uint2 dstTexCoord = uint2(GlobalInvocationID.xy);
  const float4 nonCoveredColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  const int scalar_offset = (16u) / 4;
    bool tint_tmp_2 = (dstTexCoord.x < uniforms[scalar_offset / 4][scalar_offset % 4]);
  if (!tint_tmp_2) {
const int scalar_offset_1 = (20u) / 4;
        tint_tmp_2 = (dstTexCoord.y < uniforms[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  }
  bool tint_tmp_3 = (tint_tmp_2);
  if (!tint_tmp_3) {
const int scalar_offset_2 = (16u) / 4;
    const int scalar_offset_3 = (24u) / 4;
        tint_tmp_3 = (dstTexCoord.x >= (uniforms[scalar_offset_2 / 4][scalar_offset_2 % 4] + uniforms[scalar_offset_3 / 4][scalar_offset_3 % 4]));
  }
  bool tint_tmp_4 = (tint_tmp_3);
  if (!tint_tmp_4) {
const int scalar_offset_4 = (20u) / 4;
    const int scalar_offset_5 = (28u) / 4;
        tint_tmp_4 = (dstTexCoord.y >= (uniforms[scalar_offset_4 / 4][scalar_offset_4 % 4] + uniforms[scalar_offset_5 / 4][scalar_offset_5 % 4]));
  }
if ((tint_tmp_4)) {
        bool tint_tmp_5 = success;
    if (tint_tmp_5) {
      tint_tmp_5 = all((tint_symbol.Load(int3(dstTexCoord, 0), 0) == nonCoveredColor));
    }
success = (tint_tmp_5);
  } else {
    const int scalar_offset_6 = (16u) / 4;
    uint4 ubo_load = uniforms[scalar_offset_6 / 4];
    const int scalar_offset_7 = (8u) / 4;
    uint4 ubo_load_1 = uniforms[scalar_offset_7 / 4];
    uint2 srcTexCoord = ((dstTexCoord - ((scalar_offset_6 & 2) ? ubo_load.zw : ubo_load.xy)) + ((scalar_offset_7 & 2) ? ubo_load_1.zw : ubo_load_1.xy));
    const int scalar_offset_8 = (0u) / 4;
    if ((uniforms[scalar_offset_8 / 4][scalar_offset_8 % 4] == 1u)) {
      srcTexCoord.y = ((uint(srcSize.y) - srcTexCoord.y) - 1u);
    }
    const float4 srcColor = src.Load(int3(srcTexCoord, 0), 0);
    const float4 dstColor = tint_symbol.Load(int3(dstTexCoord, 0), 0);
    const int scalar_offset_9 = (4u) / 4;
    if ((uniforms[scalar_offset_9 / 4][scalar_offset_9 % 4] == 2u)) {
            bool tint_tmp_6 = success;
      if (tint_tmp_6) {
        tint_tmp_6 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_tmp_7 = (tint_tmp_6);
      if (tint_tmp_7) {
        tint_tmp_7 = aboutEqual(dstColor.g, srcColor.g);
      }
success = (tint_tmp_7);
    } else {
            bool tint_tmp_8 = success;
      if (tint_tmp_8) {
        tint_tmp_8 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_tmp_9 = (tint_tmp_8);
      if (tint_tmp_9) {
        tint_tmp_9 = aboutEqual(dstColor.g, srcColor.g);
      }
      bool tint_tmp_10 = (tint_tmp_9);
      if (tint_tmp_10) {
        tint_tmp_10 = aboutEqual(dstColor.b, srcColor.b);
      }
      bool tint_tmp_11 = (tint_tmp_10);
      if (tint_tmp_11) {
        tint_tmp_11 = aboutEqual(dstColor.a, srcColor.a);
      }
success = (tint_tmp_11);
    }
  }
  const uint outputIndex = ((GlobalInvocationID.y * uint(dstSize.x)) + GlobalInvocationID.x);
  if (success) {
    output.Store((4u * outputIndex), asuint(1u));
  } else {
    output.Store((4u * outputIndex), asuint(0u));
  }
  return;
}
