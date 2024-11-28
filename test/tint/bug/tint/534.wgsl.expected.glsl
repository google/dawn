#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
};

struct TintTextureUniformData {
  uint tint_builtin_value_0;
  uint tint_builtin_value_1;
};

layout(binding = 2, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} tint_symbol;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp sampler2D src;
uniform highp sampler2D dst;
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  return mix(uvec4(4294967295u), mix(uvec4(0u), uvec4(value), greaterThanEqual(value, vec4(0.0f))), lessThanEqual(value, vec4(4294967040.0f)));
}
void tint_symbol_1_inner(uvec3 GlobalInvocationID) {
  uvec2 size = uvec2(textureSize(src, 0));
  uvec2 dstTexCoord = GlobalInvocationID.xy;
  uvec2 srcTexCoord = dstTexCoord;
  if ((v.inner.dstTextureFlipY == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  uvec2 v_2 = srcTexCoord;
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(0), v_3);
  ivec2 v_5 = ivec2(min(v_2, (uvec2(textureSize(src, int(v_4))) - uvec2(1u))));
  vec4 srcColor = texelFetch(src, v_5, int(v_4));
  uvec2 v_6 = dstTexCoord;
  uint v_7 = (v_1.inner.tint_builtin_value_1 - 1u);
  uint v_8 = min(uint(0), v_7);
  ivec2 v_9 = ivec2(min(v_6, (uvec2(textureSize(dst, int(v_8))) - uvec2(1u))));
  vec4 dstColor = texelFetch(dst, v_9, int(v_8));
  bool success = true;
  uvec4 srcColorBits = uvec4(0u);
  uvec4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uint i = 0u;
    while(true) {
      if ((i < v.inner.channelCount)) {
      } else {
        break;
      }
      uint v_10 = i;
      srcColorBits[min(v_10, 3u)] = ConvertToFp16FloatValue(srcColor[min(i, 3u)]);
      bool v_11 = false;
      if (success) {
        v_11 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      } else {
        v_11 = false;
      }
      success = v_11;
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    uint v_12 = outputIndex;
    uint v_13 = min(v_12, (uint(tint_symbol.result.length()) - 1u));
    tint_symbol.result[v_13] = 1u;
  } else {
    uint v_14 = outputIndex;
    uint v_15 = min(v_14, (uint(tint_symbol.result.length()) - 1u));
    tint_symbol.result[v_15] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_GlobalInvocationID);
}
