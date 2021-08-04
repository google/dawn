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

void main_inner(uint3 GlobalInvocationID) {
  int2 tint_tmp;
  src.GetDimensions(tint_tmp.x, tint_tmp.y);
  const int2 srcSize = tint_tmp;
  int2 tint_tmp_1;
  tint_symbol.GetDimensions(tint_tmp_1.x, tint_tmp_1.y);
  const int2 dstSize = tint_tmp_1;
  const uint2 dstTexCoord = uint2(GlobalInvocationID.xy);
  const float4 nonCoveredColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool tint_tmp_4 = (dstTexCoord.x < uniforms[1].x);
  if (!tint_tmp_4) {
    tint_tmp_4 = (dstTexCoord.y < uniforms[1].y);
  }
  bool tint_tmp_3 = (tint_tmp_4);
  if (!tint_tmp_3) {
    tint_tmp_3 = (dstTexCoord.x >= (uniforms[1].x + uniforms[1].z));
  }
  bool tint_tmp_2 = (tint_tmp_3);
  if (!tint_tmp_2) {
    tint_tmp_2 = (dstTexCoord.y >= (uniforms[1].y + uniforms[1].w));
  }
  if ((tint_tmp_2)) {
    bool tint_tmp_5 = success;
    if (tint_tmp_5) {
      tint_tmp_5 = all((tint_symbol.Load(int3(int2(dstTexCoord), 0)) == nonCoveredColor));
    }
    success = (tint_tmp_5);
  } else {
    uint2 srcTexCoord = ((dstTexCoord - uniforms[1].xy) + uniforms[0].zw);
    if ((uniforms[0].x == 1u)) {
      srcTexCoord.y = ((uint(srcSize.y) - srcTexCoord.y) - 1u);
    }
    const float4 srcColor = src.Load(int3(int2(srcTexCoord), 0));
    const float4 dstColor = tint_symbol.Load(int3(int2(dstTexCoord), 0));
    if ((uniforms[0].y == 2u)) {
      bool tint_tmp_7 = success;
      if (tint_tmp_7) {
        tint_tmp_7 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_tmp_6 = (tint_tmp_7);
      if (tint_tmp_6) {
        tint_tmp_6 = aboutEqual(dstColor.g, srcColor.g);
      }
      success = (tint_tmp_6);
    } else {
      bool tint_tmp_11 = success;
      if (tint_tmp_11) {
        tint_tmp_11 = aboutEqual(dstColor.r, srcColor.r);
      }
      bool tint_tmp_10 = (tint_tmp_11);
      if (tint_tmp_10) {
        tint_tmp_10 = aboutEqual(dstColor.g, srcColor.g);
      }
      bool tint_tmp_9 = (tint_tmp_10);
      if (tint_tmp_9) {
        tint_tmp_9 = aboutEqual(dstColor.b, srcColor.b);
      }
      bool tint_tmp_8 = (tint_tmp_9);
      if (tint_tmp_8) {
        tint_tmp_8 = aboutEqual(dstColor.a, srcColor.a);
      }
      success = (tint_tmp_8);
    }
  }
  const uint outputIndex = ((GlobalInvocationID.y * uint(dstSize.x)) + GlobalInvocationID.x);
  if (success) {
    output.Store((4u * outputIndex), asuint(1u));
  } else {
    output.Store((4u * outputIndex), asuint(0u));
  }
}

[numthreads(1, 1, 1)]
void main(tint_symbol_2 tint_symbol_1) {
  main_inner(tint_symbol_1.GlobalInvocationID);
  return;
}
