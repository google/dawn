struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2D<float4> src : register(t0);
Texture2D<float4> tint_symbol : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};
bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}

void main_inner(uint3 GlobalInvocationID) {
  uint2 v = (0u).xx;
  src.GetDimensions(v.x, v.y);
  uint2 srcSize = v;
  uint2 v_1 = (0u).xx;
  tint_symbol.GetDimensions(v_1.x, v_1.y);
  uint2 dstSize = v_1;
  uint2 dstTexCoord = uint2(GlobalInvocationID.xy);
  float4 nonCoveredColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool v_2 = false;
  if ((dstTexCoord.x < uniforms[1u].x)) {
    v_2 = true;
  } else {
    v_2 = (dstTexCoord.y < uniforms[1u].y);
  }
  bool v_3 = false;
  if (v_2) {
    v_3 = true;
  } else {
    v_3 = (dstTexCoord.x >= (uniforms[1u].x + uniforms[1u].z));
  }
  bool v_4 = false;
  if (v_3) {
    v_4 = true;
  } else {
    v_4 = (dstTexCoord.y >= (uniforms[1u].y + uniforms[1u].w));
  }
  if (v_4) {
    bool v_5 = false;
    if (success) {
      int2 v_6 = int2(dstTexCoord);
      uint3 v_7 = (0u).xxx;
      tint_symbol.GetDimensions(0u, v_7.x, v_7.y, v_7.z);
      uint v_8 = min(uint(int(0)), (v_7.z - 1u));
      uint3 v_9 = (0u).xxx;
      tint_symbol.GetDimensions(uint(v_8), v_9.x, v_9.y, v_9.z);
      uint2 v_10 = (v_9.xy - (1u).xx);
      int2 v_11 = int2(min(uint2(v_6), v_10));
      v_5 = all((float4(tint_symbol.Load(int3(v_11, int(v_8)))) == nonCoveredColor));
    } else {
      v_5 = false;
    }
    success = v_5;
  } else {
    uint2 srcTexCoord = ((dstTexCoord - uniforms[1u].xy) + uniforms[0u].zw);
    if ((uniforms[0u].x == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    int2 v_12 = int2(srcTexCoord);
    uint3 v_13 = (0u).xxx;
    src.GetDimensions(0u, v_13.x, v_13.y, v_13.z);
    uint v_14 = min(uint(int(0)), (v_13.z - 1u));
    uint3 v_15 = (0u).xxx;
    src.GetDimensions(uint(v_14), v_15.x, v_15.y, v_15.z);
    uint2 v_16 = (v_15.xy - (1u).xx);
    int2 v_17 = int2(min(uint2(v_12), v_16));
    float4 srcColor = float4(src.Load(int3(v_17, int(v_14))));
    int2 v_18 = int2(dstTexCoord);
    uint3 v_19 = (0u).xxx;
    tint_symbol.GetDimensions(0u, v_19.x, v_19.y, v_19.z);
    uint v_20 = min(uint(int(0)), (v_19.z - 1u));
    uint3 v_21 = (0u).xxx;
    tint_symbol.GetDimensions(uint(v_20), v_21.x, v_21.y, v_21.z);
    uint2 v_22 = (v_21.xy - (1u).xx);
    int2 v_23 = int2(min(uint2(v_18), v_22));
    float4 dstColor = float4(tint_symbol.Load(int3(v_23, int(v_20))));
    if ((uniforms[0u].y == 2u)) {
      bool v_24 = false;
      if (success) {
        v_24 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_24 = false;
      }
      bool v_25 = false;
      if (v_24) {
        v_25 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_25 = false;
      }
      success = v_25;
    } else {
      bool v_26 = false;
      if (success) {
        v_26 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_26 = false;
      }
      bool v_27 = false;
      if (v_26) {
        v_27 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_27 = false;
      }
      bool v_28 = false;
      if (v_27) {
        v_28 = aboutEqual(dstColor.z, srcColor.z);
      } else {
        v_28 = false;
      }
      bool v_29 = false;
      if (v_28) {
        v_29 = aboutEqual(dstColor.w, srcColor.w);
      } else {
        v_29 = false;
      }
      success = v_29;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint v_30 = 0u;
    output.GetDimensions(v_30);
    output.Store((0u + (min(outputIndex, ((v_30 / 4u) - 1u)) * 4u)), 1u);
  } else {
    uint v_31 = 0u;
    output.GetDimensions(v_31);
    output.Store((0u + (min(outputIndex, ((v_31 / 4u) - 1u)) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

