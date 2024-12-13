struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2D<float4> src : register(t0);
Texture2D<float4> v : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};
bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}

void main_inner(uint3 GlobalInvocationID) {
  uint2 v_1 = (0u).xx;
  src.GetDimensions(v_1.x, v_1.y);
  uint2 srcSize = v_1;
  uint2 v_2 = (0u).xx;
  v.GetDimensions(v_2.x, v_2.y);
  uint2 dstSize = v_2;
  uint2 dstTexCoord = uint2(GlobalInvocationID.xy);
  float4 nonCoveredColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool v_3 = false;
  if ((dstTexCoord.x < uniforms[1u].x)) {
    v_3 = true;
  } else {
    v_3 = (dstTexCoord.y < uniforms[1u].y);
  }
  bool v_4 = false;
  if (v_3) {
    v_4 = true;
  } else {
    v_4 = (dstTexCoord.x >= (uniforms[1u].x + uniforms[1u].z));
  }
  bool v_5 = false;
  if (v_4) {
    v_5 = true;
  } else {
    v_5 = (dstTexCoord.y >= (uniforms[1u].y + uniforms[1u].w));
  }
  if (v_5) {
    bool v_6 = false;
    if (success) {
      int2 v_7 = int2(dstTexCoord);
      uint3 v_8 = (0u).xxx;
      v.GetDimensions(0u, v_8.x, v_8.y, v_8.z);
      uint v_9 = min(uint(int(0)), (v_8.z - 1u));
      uint3 v_10 = (0u).xxx;
      v.GetDimensions(uint(v_9), v_10.x, v_10.y, v_10.z);
      uint2 v_11 = (v_10.xy - (1u).xx);
      int2 v_12 = int2(min(uint2(v_7), v_11));
      v_6 = all((float4(v.Load(int3(v_12, int(v_9)))) == nonCoveredColor));
    } else {
      v_6 = false;
    }
    success = v_6;
  } else {
    uint2 srcTexCoord = ((dstTexCoord - uniforms[1u].xy) + uniforms[0u].zw);
    if ((uniforms[0u].x == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    int2 v_13 = int2(srcTexCoord);
    uint3 v_14 = (0u).xxx;
    src.GetDimensions(0u, v_14.x, v_14.y, v_14.z);
    uint v_15 = min(uint(int(0)), (v_14.z - 1u));
    uint3 v_16 = (0u).xxx;
    src.GetDimensions(uint(v_15), v_16.x, v_16.y, v_16.z);
    uint2 v_17 = (v_16.xy - (1u).xx);
    int2 v_18 = int2(min(uint2(v_13), v_17));
    float4 srcColor = float4(src.Load(int3(v_18, int(v_15))));
    int2 v_19 = int2(dstTexCoord);
    uint3 v_20 = (0u).xxx;
    v.GetDimensions(0u, v_20.x, v_20.y, v_20.z);
    uint v_21 = min(uint(int(0)), (v_20.z - 1u));
    uint3 v_22 = (0u).xxx;
    v.GetDimensions(uint(v_21), v_22.x, v_22.y, v_22.z);
    uint2 v_23 = (v_22.xy - (1u).xx);
    int2 v_24 = int2(min(uint2(v_19), v_23));
    float4 dstColor = float4(v.Load(int3(v_24, int(v_21))));
    if ((uniforms[0u].y == 2u)) {
      bool v_25 = false;
      if (success) {
        v_25 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_25 = false;
      }
      bool v_26 = false;
      if (v_25) {
        v_26 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_26 = false;
      }
      success = v_26;
    } else {
      bool v_27 = false;
      if (success) {
        v_27 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_27 = false;
      }
      bool v_28 = false;
      if (v_27) {
        v_28 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_28 = false;
      }
      bool v_29 = false;
      if (v_28) {
        v_29 = aboutEqual(dstColor.z, srcColor.z);
      } else {
        v_29 = false;
      }
      bool v_30 = false;
      if (v_29) {
        v_30 = aboutEqual(dstColor.w, srcColor.w);
      } else {
        v_30 = false;
      }
      success = v_30;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint v_31 = 0u;
    output.GetDimensions(v_31);
    output.Store((0u + (min(outputIndex, ((v_31 / 4u) - 1u)) * 4u)), 1u);
  } else {
    uint v_32 = 0u;
    output.GetDimensions(v_32);
    output.Store((0u + (min(outputIndex, ((v_32 / 4u) - 1u)) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

