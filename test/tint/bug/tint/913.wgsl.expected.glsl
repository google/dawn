#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint channelCount;
  uvec2 srcCopyOrigin;
  uvec2 dstCopyOrigin;
  uvec2 copySize;
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
bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}
void tint_symbol_1_inner(uvec3 GlobalInvocationID) {
  uvec2 srcSize = uvec2(textureSize(src, 0));
  uvec2 dstSize = uvec2(textureSize(dst, 0));
  uvec2 dstTexCoord = uvec2(GlobalInvocationID.xy);
  vec4 nonCoveredColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  bool v_2 = false;
  if ((dstTexCoord[0u] < v.inner.dstCopyOrigin.x)) {
    v_2 = true;
  } else {
    v_2 = (dstTexCoord[1u] < v.inner.dstCopyOrigin.y);
  }
  bool v_3 = false;
  if (v_2) {
    v_3 = true;
  } else {
    v_3 = (dstTexCoord[0u] >= (v.inner.dstCopyOrigin.x + v.inner.copySize.x));
  }
  bool v_4 = false;
  if (v_3) {
    v_4 = true;
  } else {
    v_4 = (dstTexCoord[1u] >= (v.inner.dstCopyOrigin.y + v.inner.copySize.y));
  }
  if (v_4) {
    bool v_5 = false;
    if (success) {
      ivec2 v_6 = ivec2(dstTexCoord);
      uint v_7 = (v_1.inner.tint_builtin_value_1 - 1u);
      uint v_8 = min(uint(0), v_7);
      uvec2 v_9 = (uvec2(textureSize(dst, int(v_8))) - uvec2(1u));
      ivec2 v_10 = ivec2(min(uvec2(v_6), v_9));
      v_5 = all(equal(texelFetch(dst, v_10, int(v_8)), nonCoveredColor));
    } else {
      v_5 = false;
    }
    success = v_5;
  } else {
    uvec2 srcTexCoord = ((dstTexCoord - v.inner.dstCopyOrigin) + v.inner.srcCopyOrigin);
    if ((v.inner.dstTextureFlipY == 1u)) {
      srcTexCoord[1u] = ((srcSize[1u] - srcTexCoord.y) - 1u);
    }
    ivec2 v_11 = ivec2(srcTexCoord);
    uint v_12 = (v_1.inner.tint_builtin_value_0 - 1u);
    uint v_13 = min(uint(0), v_12);
    uvec2 v_14 = (uvec2(textureSize(src, int(v_13))) - uvec2(1u));
    ivec2 v_15 = ivec2(min(uvec2(v_11), v_14));
    vec4 srcColor = texelFetch(src, v_15, int(v_13));
    ivec2 v_16 = ivec2(dstTexCoord);
    uint v_17 = (v_1.inner.tint_builtin_value_1 - 1u);
    uint v_18 = min(uint(0), v_17);
    uvec2 v_19 = (uvec2(textureSize(dst, int(v_18))) - uvec2(1u));
    ivec2 v_20 = ivec2(min(uvec2(v_16), v_19));
    vec4 dstColor = texelFetch(dst, v_20, int(v_18));
    if ((v.inner.channelCount == 2u)) {
      bool v_21 = false;
      if (success) {
        v_21 = aboutEqual(dstColor[0u], srcColor[0u]);
      } else {
        v_21 = false;
      }
      bool v_22 = false;
      if (v_21) {
        v_22 = aboutEqual(dstColor[1u], srcColor[1u]);
      } else {
        v_22 = false;
      }
      success = v_22;
    } else {
      bool v_23 = false;
      if (success) {
        v_23 = aboutEqual(dstColor[0u], srcColor[0u]);
      } else {
        v_23 = false;
      }
      bool v_24 = false;
      if (v_23) {
        v_24 = aboutEqual(dstColor[1u], srcColor[1u]);
      } else {
        v_24 = false;
      }
      bool v_25 = false;
      if (v_24) {
        v_25 = aboutEqual(dstColor[2u], srcColor[2u]);
      } else {
        v_25 = false;
      }
      bool v_26 = false;
      if (v_25) {
        v_26 = aboutEqual(dstColor[3u], srcColor[3u]);
      } else {
        v_26 = false;
      }
      success = v_26;
    }
  }
  uint outputIndex = ((GlobalInvocationID[1u] * dstSize[0u]) + GlobalInvocationID[0u]);
  if (success) {
    uint v_27 = min(outputIndex, (uint(tint_symbol.result.length()) - 1u));
    tint_symbol.result[v_27] = 1u;
  } else {
    uint v_28 = min(outputIndex, (uint(tint_symbol.result.length()) - 1u));
    tint_symbol.result[v_28] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_GlobalInvocationID);
}
