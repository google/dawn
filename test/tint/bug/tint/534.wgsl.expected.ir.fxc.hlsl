struct main_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


Texture2D<float4> src : register(t0);
Texture2D<float4> tint_symbol : register(t1);
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
  uint2 v = (0u).xx;
  src.GetDimensions(v.x, v.y);
  uint2 size = v;
  uint2 dstTexCoord = GlobalInvocationID.xy;
  uint2 srcTexCoord = dstTexCoord;
  if ((uniforms[0u].x == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  uint2 v_1 = srcTexCoord;
  uint3 v_2 = (0u).xxx;
  src.GetDimensions(0u, v_2.x, v_2.y, v_2.z);
  uint v_3 = min(uint(int(0)), (v_2.z - 1u));
  uint3 v_4 = (0u).xxx;
  src.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z);
  int2 v_5 = int2(min(v_1, (v_4.xy - (1u).xx)));
  float4 srcColor = float4(src.Load(int3(v_5, int(v_3))));
  uint2 v_6 = dstTexCoord;
  uint3 v_7 = (0u).xxx;
  tint_symbol.GetDimensions(0u, v_7.x, v_7.y, v_7.z);
  uint v_8 = min(uint(int(0)), (v_7.z - 1u));
  uint3 v_9 = (0u).xxx;
  tint_symbol.GetDimensions(uint(v_8), v_9.x, v_9.y, v_9.z);
  int2 v_10 = int2(min(v_6, (v_9.xy - (1u).xx)));
  float4 dstColor = float4(tint_symbol.Load(int3(v_10, int(v_8))));
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
      uint v_11 = i;
      uint v_12 = ConvertToFp16FloatValue(srcColor[min(i, 3u)]);
      uint4 v_13 = srcColorBits;
      srcColorBits = (((v_11.xxxx == uint4(int(0), int(1), int(2), int(3)))) ? (v_12.xxxx) : (v_13));
      bool v_14 = false;
      if (success) {
        v_14 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      } else {
        v_14 = false;
      }
      success = v_14;
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
    uint v_15 = 0u;
    output.GetDimensions(v_15);
    output.Store((0u + (min(outputIndex, ((v_15 / 4u) - 1u)) * 4u)), 1u);
  } else {
    uint v_16 = 0u;
    output.GetDimensions(v_16);
    output.Store((0u + (min(outputIndex, ((v_16 / 4u) - 1u)) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

