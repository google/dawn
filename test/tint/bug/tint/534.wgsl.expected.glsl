#version 310 es

layout(binding = 3, std430)
buffer OutputBuf_1_ssbo {
  uint result[];
} v;
layout(binding = 2, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v_1;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_2;
uniform highp sampler2D src;
uniform highp sampler2D dst;
uint ConvertToFp16FloatValue(float fp32) {
  return 1u;
}
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  return uvec4(clamp(value, vec4(0.0f), vec4(4294967040.0f)));
}
void main_inner(uvec3 GlobalInvocationID) {
  uvec2 size = uvec2(textureSize(src, 0));
  uvec2 dstTexCoord = GlobalInvocationID.xy;
  uvec2 srcTexCoord = dstTexCoord;
  uvec4 v_3 = v_1.inner[0u];
  if ((v_3.x == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1u);
  }
  uvec2 v_4 = srcTexCoord;
  uint v_5 = (v_2.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_6 = min(uint(0), v_5);
  ivec2 v_7 = ivec2(min(v_4, (uvec2(textureSize(src, int(v_6))) - uvec2(1u))));
  vec4 srcColor = texelFetch(src, v_7, int(v_6));
  uvec2 v_8 = dstTexCoord;
  uint v_9 = (v_2.metadata[(1u / 4u)][(1u % 4u)] - 1u);
  uint v_10 = min(uint(0), v_9);
  ivec2 v_11 = ivec2(min(v_8, (uvec2(textureSize(dst, int(v_10))) - uvec2(1u))));
  vec4 dstColor = texelFetch(dst, v_11, int(v_10));
  bool success = true;
  uvec4 srcColorBits = uvec4(0u);
  uvec4 dstColorBits = tint_v4f32_to_v4u32(dstColor);
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    uint i = 0u;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      uvec4 v_12 = v_1.inner[0u];
      if ((i < v_12.w)) {
      } else {
        break;
      }
      uint v_13 = i;
      srcColorBits[min(v_13, 3u)] = ConvertToFp16FloatValue(srcColor[min(i, 3u)]);
      bool v_14 = false;
      if (success) {
        v_14 = (srcColorBits[min(i, 3u)] == dstColorBits[min(i, 3u)]);
      } else {
        v_14 = false;
      }
      success = v_14;
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        i = (i + 1u);
      }
      continue;
    }
  }
  uint outputIndex = ((GlobalInvocationID.y * uint(size.x)) + GlobalInvocationID.x);
  if (success) {
    uint v_15 = outputIndex;
    uint v_16 = min(v_15, (uint(v.result.length()) - 1u));
    v.result[v_16] = 1u;
  } else {
    uint v_17 = outputIndex;
    uint v_18 = min(v_17, (uint(v.result.length()) - 1u));
    v.result[v_18] = 0u;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
