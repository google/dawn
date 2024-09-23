#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint isFloat16;
  uint isRGB10A2Unorm;
  uint channelCount;
};

uniform highp sampler2D src;
uniform highp sampler2D dst;
layout(binding = 2, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} tint_symbol;
layout(binding = 3, std140)
uniform tint_symbol_3_1_ubo {
  Uniforms tint_symbol_2;
} v;
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  uvec4 v_1 = uvec4(value);
  bvec4 v_2 = greaterThanEqual(value, vec4(0.0f));
  uint v_3 = ((v_2.x) ? (v_1.x) : (uvec4(0u).x));
  uint v_4 = ((v_2.y) ? (v_1.y) : (uvec4(0u).y));
  uint v_5 = ((v_2.z) ? (v_1.z) : (uvec4(0u).z));
  uvec4 v_6 = uvec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (uvec4(0u).w)));
  bvec4 v_7 = lessThanEqual(value, vec4(4294967040.0f));
  uint v_8 = ((v_7.x) ? (v_6.x) : (uvec4(4294967295u).x));
  uint v_9 = ((v_7.y) ? (v_6.y) : (uvec4(4294967295u).y));
  uint v_10 = ((v_7.z) ? (v_6.z) : (uvec4(4294967295u).z));
  return uvec4(v_8, v_9, v_10, ((v_7.w) ? (v_6.w) : (uvec4(4294967295u).w)));
}
void tint_symbol_1_inner(uvec3 GlobalInvocationID) {
  uvec2 size = uvec2(textureSize(src, 0));
  uvec2 dstTexCoord = GlobalInvocationID.xy;
  uvec2 srcTexCoord = dstTexCoord;
  if ((v.tint_symbol_2.dstTextureFlipY == 1u)) {
    srcTexCoord[1u] = ((size.y - dstTexCoord.y) - 1u);
  }
  ivec2 v_11 = ivec2(srcTexCoord);
  vec4 srcColor = texelFetch(src, v_11, int(0));
  ivec2 v_12 = ivec2(dstTexCoord);
  vec4 dstColor = texelFetch(dst, v_12, int(0));
  bool success = true;
  uvec4 srcColorBits = uvec4(0u);
  uvec4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uint i = 0u;
    while(true) {
      if ((i < v.tint_symbol_2.channelCount)) {
      } else {
        break;
      }
      uint v_13 = i;
      srcColorBits[v_13] = ConvertToFp16FloatValue(srcColor[i]);
      bool v_14 = false;
      if (success) {
        v_14 = (srcColorBits[i] == dstColorBits[i]);
      } else {
        v_14 = false;
      }
      success = v_14;
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID[1u] * uint(size.x)) + GlobalInvocationID[0u]);
  if (success) {
    tint_symbol.result[outputIndex] = 1u;
  } else {
    tint_symbol.result[outputIndex] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_GlobalInvocationID);
}
