#version 310 es

layout(binding = 3, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} v;
layout(binding = 2, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[2];
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
  uvec4 v_3 = v_1.inner[1u];
  bool v_4 = false;
  if ((dstTexCoord.x < v_3.x)) {
    v_4 = true;
  } else {
    uvec4 v_5 = v_1.inner[1u];
    v_4 = (dstTexCoord.y < v_5.y);
  }
  bool v_6 = false;
  if (v_4) {
    v_6 = true;
  } else {
    uvec4 v_7 = v_1.inner[1u];
    uvec4 v_8 = v_1.inner[1u];
    v_6 = (dstTexCoord.x >= (v_7.x + v_8.z));
  }
  bool v_9 = false;
  if (v_6) {
    v_9 = true;
  } else {
    uvec4 v_10 = v_1.inner[1u];
    uvec4 v_11 = v_1.inner[1u];
    v_9 = (dstTexCoord.y >= (v_10.y + v_11.w));
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
    uvec2 srcTexCoord = ((dstTexCoord - v_1.inner[1u].xy) + v_1.inner[0u].zw);
    uvec4 v_18 = v_1.inner[0u];
    if ((v_18.x == 1u)) {
      srcTexCoord.y = ((srcSize.y - srcTexCoord.y) - 1u);
    }
    ivec2 v_19 = ivec2(srcTexCoord);
    uint v_20 = (v_2.metadata[(0u / 4u)][(0u % 4u)] - 1u);
    uint v_21 = min(uint(0), v_20);
    uvec2 v_22 = (uvec2(textureSize(src, int(v_21))) - uvec2(1u));
    ivec2 v_23 = ivec2(min(uvec2(v_19), v_22));
    vec4 srcColor = texelFetch(src, v_23, int(v_21));
    ivec2 v_24 = ivec2(dstTexCoord);
    uint v_25 = (v_2.metadata[(1u / 4u)][(1u % 4u)] - 1u);
    uint v_26 = min(uint(0), v_25);
    uvec2 v_27 = (uvec2(textureSize(dst, int(v_26))) - uvec2(1u));
    ivec2 v_28 = ivec2(min(uvec2(v_24), v_27));
    vec4 dstColor = texelFetch(dst, v_28, int(v_26));
    uvec4 v_29 = v_1.inner[0u];
    if ((v_29.y == 2u)) {
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
      success = v_31;
    } else {
      bool v_32 = false;
      if (success) {
        v_32 = aboutEqual(dstColor.x, srcColor.x);
      } else {
        v_32 = false;
      }
      bool v_33 = false;
      if (v_32) {
        v_33 = aboutEqual(dstColor.y, srcColor.y);
      } else {
        v_33 = false;
      }
      bool v_34 = false;
      if (v_33) {
        v_34 = aboutEqual(dstColor.z, srcColor.z);
      } else {
        v_34 = false;
      }
      bool v_35 = false;
      if (v_34) {
        v_35 = aboutEqual(dstColor.w, srcColor.w);
      } else {
        v_35 = false;
      }
      success = v_35;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * dstSize.x) + GlobalInvocationID.x);
  if (success) {
    uint v_36 = min(outputIndex, (uint(v.result.length()) - 1u));
    v.result[v_36] = 1u;
  } else {
    uint v_37 = min(outputIndex, (uint(v.result.length()) - 1u));
    v.result[v_37] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
