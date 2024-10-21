#version 310 es


struct Params {
  uint filterDim;
  uint blockDim;
};

struct Flip {
  uint value;
};

layout(binding = 1, std140)
uniform params_block_1_ubo {
  Params inner;
} v;
layout(binding = 2, rgba8) uniform highp writeonly image2D outputTex;
layout(binding = 3, std140)
uniform flip_block_1_ubo {
  Flip inner;
} v_1;
shared vec3 tile[4][256];
uniform highp sampler2D inputTex_samp;
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void tint_symbol_inner(uvec3 WorkGroupID, uvec3 LocalInvocationID, uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 1024u)) {
        break;
      }
      tile[(v_3 / 256u)][(v_3 % 256u)] = vec3(0.0f);
      {
        v_2 = (v_3 + 64u);
      }
      continue;
    }
  }
  barrier();
  uint filterOffset = tint_div_u32((v.inner.filterDim - 1u), 2u);
  uvec2 dims = uvec2(textureSize(inputTex_samp, 0));
  uvec2 v_4 = ((WorkGroupID.xy * uvec2(v.inner.blockDim, 4u)) + (LocalInvocationID.xy * uvec2(4u, 1u)));
  uvec2 baseIndex = (v_4 - uvec2(filterOffset, 0u));
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
          if ((v_1.inner.value != 0u)) {
            loadIndex = loadIndex.yx;
          }
          uint v_5 = r;
          uint v_6 = ((4u * LocalInvocationID[0u]) + c);
          vec2 v_7 = (vec2(loadIndex) + vec2(0.25f));
          vec2 v_8 = (v_7 / vec2(dims));
          tile[v_5][v_6] = textureLod(inputTex_samp, v_8, float(0.0f)).xyz;
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
          if ((v_1.inner.value != 0u)) {
            writeIndex = writeIndex.yx;
          }
          uint center = ((4u * LocalInvocationID[0u]) + c);
          bool v_9 = false;
          if ((center >= filterOffset)) {
            v_9 = (center < (256u - filterOffset));
          } else {
            v_9 = false;
          }
          bool v_10 = false;
          if (v_9) {
            v_10 = all(lessThan(writeIndex, dims));
          } else {
            v_10 = false;
          }
          if (v_10) {
            vec3 acc = vec3(0.0f);
            {
              uint f = 0u;
              while(true) {
                if ((f < v.inner.filterDim)) {
                } else {
                  break;
                }
                uint i = ((center + f) - filterOffset);
                vec3 v_11 = acc;
                float v_12 = (1.0f / float(v.inner.filterDim));
                uint v_13 = r;
                uint v_14 = i;
                acc = (v_11 + (v_12 * tile[v_13][v_14]));
                {
                  f = (f + 1u);
                }
                continue;
              }
            }
            uvec2 v_15 = writeIndex;
            vec4 v_16 = vec4(acc, 1.0f);
            imageStore(outputTex, ivec2(v_15), v_16);
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
  tint_symbol_inner(gl_WorkGroupID, gl_LocalInvocationID, gl_LocalInvocationIndex);
}
