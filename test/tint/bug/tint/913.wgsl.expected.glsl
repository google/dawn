#version 310 es


struct Uniforms {
  uint dstTextureFlipY;
  uint channelCount;
  uvec2 srcCopyOrigin;
  uvec2 dstCopyOrigin;
  uvec2 copySize;
};

layout(binding = 2, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} v;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v_1;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_2;
uniform highp sampler2D src;
uniform highp sampler2D dst;
bool aboutEqual(float value, float expect) {
  return (abs((value - expect)) < 0.00100000004749745131f);
}
void main_inner(uvec3 GlobalInvocationID) {
  uvec2 srcSize = uvec2(textureSize(src, 0));
  uvec2 dstSize = uvec2(textureSize(dst, 0));
  uvec2 dstTexCoord = uvec2(GlobalInvocationID.xy);
  vec4 nonCoveredColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  bool success = true;
  uvec2 v_3 = v_1.inner.dstCopyOrigin;
  bool v_4 = false;
  if ((dstTexCoord.x < v_3.x)) {
    v_4 = true;
  } else {
    uvec2 v_5 = v_1.inner.dstCopyOrigin;
    v_4 = (dstTexCoord.y < v_5.y);
  }
  bool v_6 = false;
  if (v_4) {
    v_6 = true;
  } else {
    uvec2 v_7 = v_1.inner.dstCopyOrigin;
    uvec2 v_8 = v_1.inner.copySize;
    v_6 = (dstTexCoord.x >= (v_7.x + v_8.x));
  }
  bool v_9 = false;
  if (v_6) {
    v_9 = true;
  } else {
    uvec2 v_10 = v_1.inner.dstCopyOrigin;
    uvec2 v_11 = v_1.inner.copySize;
    v_9 = (dstTexCoord.y >= (v_10.y + v_11.y));
  }
  if (v_9) {
    bool v_12 = false;
    if (success) {
      ivec2 v_13 = ivec2(dstTexCoord);
      uint v_14 = (v_2.metadata[(1u / 4u)][(1u % 4u)] - 1u);
      uint v_15 = min(uint(0), v_14);
      uvec2 v_16 = (uvec2(textureSize(dst, int(v_15))) - uvec2(1u));
      ivec2 v_17 = ivec2(min(uvec2(v_13), v_16));
      v_12 = all(equal(texelFetch(dst, v_17, int(v_15)), nonCoveredColor));
    } else {
      v_12 = false;
    }
    success = v_12;
  } else {
    uvec2 srcTexCoord = ((dstTexCoord - v_1.inner.dstCopyOrigin) + v_1.inner.srcCopyOrigin);
    if ((v_1.inner.dstTextureFlipY == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    ivec2 v_18 = ivec2(srcTexCoord);
    uint v_19 = (v_2.metadata[(0u / 4u)][(0u % 4u)] - 1u);
    uint v_20 = min(uint(0), v_19);
    uvec2 v_21 = (uvec2(textureSize(src, int(v_20))) - uvec2(1u));
    ivec2 v_22 = ivec2(min(uvec2(v_18), v_21));
    vec4 srcColor = texelFetch(src, v_22, int(v_20));
    ivec2 v_23 = ivec2(dstTexCoord);
    uint v_24 = (v_2.metadata[(1u / 4u)][(1u % 4u)] - 1u);
    uint v_25 = min(uint(0), v_24);
    uvec2 v_26 = (uvec2(textureSize(dst, int(v_25))) - uvec2(1u));
    ivec2 v_27 = ivec2(min(uvec2(v_23), v_26));
    vec4 dstColor = texelFetch(dst, v_27, int(v_25));
    if ((v_1.inner.channelCount == 2u)) {
      bool v_28 = false;
      if (success) {
        v_28 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_28 = false;
      }
      bool v_29 = false;
      if (v_28) {
        v_29 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_29 = false;
      }
      success = v_29;
    } else {
      bool v_30 = false;
      if (success) {
        v_30 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_30 = false;
      }
      bool v_31 = false;
      if (v_30) {
        v_31 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_31 = false;
      }
      bool v_32 = false;
      if (v_31) {
        v_32 = aboutEqual(dstColor.z, srcColor.z);
      } else {
        v_32 = false;
      }
      bool v_33 = false;
      if (v_32) {
        v_33 = aboutEqual(dstColor.w, srcColor.w);
      } else {
        v_33 = false;
      }
      success = v_33;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint v_34 = min(outputIndex, (uint(v.result.length()) - 1u));
    v.result[v_34] = 1u;
  } else {
    uint v_35 = min(outputIndex, (uint(v.result.length()) - 1u));
    v.result[v_35] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
