SKIP: FAILED

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
  src.GetDimensions(v[0u], v[1u]);
  uint2 size = v;
  uint2 dstTexCoord = GlobalInvocationID.xy;
  uint2 srcTexCoord = dstTexCoord;
  if ((uniforms[0u].x == 1u)) {
    srcTexCoord[1u] = ((size.y - dstTexCoord.y) - 1u);
  }
  Texture2D<float4> v_1 = src;
  int2 v_2 = int2(srcTexCoord);
  float4 srcColor = float4(v_1.Load(int3(v_2, int(0))));
  Texture2D<float4> v_3 = tint_symbol;
  int2 v_4 = int2(dstTexCoord);
  float4 dstColor = float4(v_3.Load(int3(v_4, int(0))));
  bool success = true;
  uint4 srcColorBits = (0u).xxxx;
  uint4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uint i = 0u;
    while(true) {
      if ((i < uniforms[0u].w)) {
      } else {
        break;
      }
      uint v_5 = i;
      srcColorBits[v_5] = ConvertToFp16FloatValue(srcColor[i]);
      bool v_6 = false;
      if (success) {
        v_6 = (srcColorBits[i] == dstColorBits[i]);
      } else {
        v_6 = false;
      }
      success = v_6;
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID[1u] * uint(size.x)) + GlobalInvocationID[0u]);
  if (success) {
    output.Store((0u + (uint(outputIndex) * 4u)), 1u);
  } else {
    output.Store((0u + (uint(outputIndex) * 4u)), 0u);
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.GlobalInvocationID);
}

FXC validation failure:
<scrubbed_path>(40,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
