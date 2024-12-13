struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2D<float4> src : register(t0);
Texture2D<float4> v : register(t1);
RWByteAddressBuffer output : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[1];
};
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}

uint4 tint_v4f32_to_v4u32(float4 value) {
  return (((value <= (4294967040.0f).xxxx)) ? ((((value >= (0.0f).xxxx)) ? (uint4(value)) : ((0u).xxxx))) : ((4294967295u).xxxx));
}

void main_inner(uint3 GlobalInvocationID) {
  uint2 v_1 = (0u).xx;
  src.GetDimensions(v_1.x, v_1.y);
  uint2 size = v_1;
  uint2 dstTexCoord = GlobalInvocationID.xy;
  uint2 srcTexCoord = dstTexCoord;
  if ((uniforms[0u].x == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  uint2 v_2 = srcTexCoord;
  uint3 v_3 = (0u).xxx;
  src.GetDimensions(0u, v_3.x, v_3.y, v_3.z);
  uint v_4 = min(uint(int(0)), (v_3.z - 1u));
  uint3 v_5 = (0u).xxx;
  src.GetDimensions(uint(v_4), v_5.x, v_5.y, v_5.z);
  int2 v_6 = int2(min(v_2, (v_5.xy - (1u).xx)));
  float4 srcColor = float4(src.Load(int3(v_6, int(v_4))));
  uint2 v_7 = dstTexCoord;
  uint3 v_8 = (0u).xxx;
  v.GetDimensions(0u, v_8.x, v_8.y, v_8.z);
  uint v_9 = min(uint(int(0)), (v_8.z - 1u));
  uint3 v_10 = (0u).xxx;
  v.GetDimensions(uint(v_9), v_10.x, v_10.y, v_10.z);
  int2 v_11 = int2(min(v_7, (v_10.xy - (1u).xx)));
  float4 dstColor = float4(v.Load(int3(v_11, int(v_9))));
  bool success = true;
  uint4 srcColorBits = (0u).xxxx;
  uint4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uint2 tint_loop_idx = (0u).xx;
    uint i = 0u;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < uniforms[0u].w)) {
      } else {
        break;
      }
      uint v_12 = i;
      uint v_13 = ConvertToFp16FloatValue(srcColor[min(i, 3u)]);
      uint4 v_14 = srcColorBits;
      srcColorBits = (((v_12.xxxx == uint4(int(0), int(1), int(2), int(3)))) ? (v_13.xxxx) : (v_14));
      bool v_15 = false;
      if (success) {
        v_15 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      } else {
        v_15 = false;
      }
      success = v_15;
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    uint v_16 = 0u;
    output.GetDimensions(v_16);
    output.Store((0u + (min(outputIndex, ((v_16 / 4u) - 1u)) * 4u)), 1u);
  } else {
    uint v_17 = 0u;
    output.GetDimensions(v_17);
    output.Store((0u + (min(outputIndex, ((v_17 / 4u) - 1u)) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

