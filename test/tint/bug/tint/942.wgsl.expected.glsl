#version 310 es

layout(binding = 0, std140)
uniform params_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 4, rgba8) uniform highp writeonly image2D outputTex;
layout(binding = 3, std140)
uniform flip_block_1_ubo {
  uvec4 inner[1];
} v_1;
shared vec3 tile[4][256];
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_2;
uniform highp sampler2D inputTex_samp;
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void main_inner(uvec3 WorkGroupID, uvec3 LocalInvocationID, uint tint_local_index) {
  {
    uint v_3 = 0u;
    v_3 = tint_local_index;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 1024u)) {
        break;
      }
      tile[(v_4 / 256u)][(v_4 % 256u)] = vec3(0.0f);
      {
        v_3 = (v_4 + 64u);
      }
      continue;
    }
  }
  barrier();
  uvec4 v_5 = v.inner[0u];
  uint filterOffset = tint_div_u32((v_5.x - 1u), 2u);
  uint v_6 = (v_2.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uvec2 dims = uvec2(textureSize(inputTex_samp, int(min(uint(0), v_6))));
  uvec4 v_7 = v.inner[0u];
  uvec2 v_8 = ((WorkGroupID.xy * uvec2(v_7.y, 4u)) + (LocalInvocationID.xy * uvec2(4u, 1u)));
  uvec2 baseIndex = (v_8 - uvec2(filterOffset, 0u));
  {
    uint r = 0u;
    while(true) {
      if ((r < 4u)) {
      } else {
        break;
      }
      {
        uint c = 0u;
        while(true) {
          if ((c < 4u)) {
          } else {
            break;
          }
          uvec2 loadIndex = (baseIndex + uvec2(c, r));
          uvec4 v_9 = v_1.inner[0u];
          if ((v_9.x != 0u)) {
            loadIndex = loadIndex.yx;
          }
          uint v_10 = r;
          uint v_11 = ((4u * LocalInvocationID.x) + c);
          vec2 v_12 = (vec2(loadIndex) + vec2(0.25f));
          tile[v_10][v_11] = textureLod(inputTex_samp, (v_12 / vec2(dims)), 0.0f).xyz;
          {
            c = (c + 1u);
          }
          continue;
        }
      }
      {
        r = (r + 1u);
      }
      continue;
    }
  }
  barrier();
  {
    uint r = 0u;
    while(true) {
      if ((r < 4u)) {
      } else {
        break;
      }
      {
        uint c = 0u;
        while(true) {
          if ((c < 4u)) {
          } else {
            break;
          }
          uvec2 writeIndex = (baseIndex + uvec2(c, r));
          uvec4 v_13 = v_1.inner[0u];
          if ((v_13.x != 0u)) {
            writeIndex = writeIndex.yx;
          }
          uint center = ((4u * LocalInvocationID.x) + c);
          bool v_14 = false;
          if ((center >= filterOffset)) {
            v_14 = (center < (256u - filterOffset));
          } else {
            v_14 = false;
          }
          bool v_15 = false;
          if (v_14) {
            v_15 = all(lessThan(writeIndex, dims));
          } else {
            v_15 = false;
          }
          if (v_15) {
            vec3 acc = vec3(0.0f);
            {
              uvec2 tint_loop_idx = uvec2(4294967295u);
              uint f = 0u;
              while(true) {
                if (all(equal(tint_loop_idx, uvec2(0u)))) {
                  break;
                }
                uvec4 v_16 = v.inner[0u];
                if ((f < v_16.x)) {
                } else {
                  break;
                }
                uint i = ((center + f) - filterOffset);
                vec3 v_17 = acc;
                uvec4 v_18 = v.inner[0u];
                float v_19 = (1.0f / float(v_18.x));
                uint v_20 = r;
                uint v_21 = min(i, 255u);
                acc = (v_17 + (v_19 * tile[v_20][v_21]));
                {
                  uint tint_low_inc = (tint_loop_idx.x - 1u);
                  tint_loop_idx.x = tint_low_inc;
                  uint tint_carry = uint((tint_low_inc == 4294967295u));
                  tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
                  f = (f + 1u);
                }
                continue;
              }
            }
            uvec2 v_22 = writeIndex;
            vec4 v_23 = vec4(acc, 1.0f);
            imageStore(outputTex, ivec2(v_22), v_23);
          }
          {
            c = (c + 1u);
          }
          continue;
        }
      }
      {
        r = (r + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_WorkGroupID, gl_LocalInvocationID, gl_LocalInvocationIndex);
}
