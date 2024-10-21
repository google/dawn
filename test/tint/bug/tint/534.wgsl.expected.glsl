#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
};

layout(binding = 2, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} tint_symbol;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
uniform highp sampler2D src;
uniform highp sampler2D dst;
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  uvec4 v_1 = uvec4(value);
  uvec4 v_2 = mix(uvec4(0u), v_1, greaterThanEqual(value, vec4(0.0f)));
  return mix(uvec4(4294967295u), v_2, lessThanEqual(value, vec4(4294967040.0f)));
}
void tint_symbol_1_inner(uvec3 GlobalInvocationID) {
  uvec2 size = uvec2(textureSize(src, 0));
  uvec2 dstTexCoord = GlobalInvocationID.xy;
  uvec2 srcTexCoord = dstTexCoord;
  if ((v.inner.dstTextureFlipY == 1u)) {
    srcTexCoord[1u] = ((size.y - dstTexCoord.y) - 1u);
  }
  ivec2 v_3 = ivec2(srcTexCoord);
  vec4 srcColor = texelFetch(src, v_3, int(0));
  ivec2 v_4 = ivec2(dstTexCoord);
  vec4 dstColor = texelFetch(dst, v_4, int(0));
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
    uint v_7 = outputIndex;
    tint_symbol.result[v_7] = 1u;
  } else {
    uint v_8 = outputIndex;
    tint_symbol.result[v_8] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_GlobalInvocationID);
}
