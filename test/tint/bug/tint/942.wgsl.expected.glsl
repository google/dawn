#version 310 es

uint tint_div(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % mix(rhs, 1u, (rhs == 0u)));
}

shared vec3 tile[4][256];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 1024u); idx = (idx + 64u)) {
      uint i_1 = tint_div(idx, 256u);
      uint i_2 = tint_mod(idx, 256u);
      tile[i_1][i_2] = vec3(0.0f);
    }
  }
  barrier();
}

struct Params {
  uint filterDim;
  uint blockDim;
};

layout(binding = 1, std140) uniform params_block_ubo {
  Params inner;
} params;

layout(binding = 2, rgba8) uniform highp writeonly image2D outputTex;
struct Flip {
  uint value;
};

layout(binding = 3, std140) uniform flip_block_ubo {
  Flip inner;
} flip;

uniform highp sampler2D inputTex_samp;

void tint_symbol(uvec3 WorkGroupID, uvec3 LocalInvocationID, uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint filterOffset = tint_div((params.inner.filterDim - 1u), 2u);
  uvec2 dims = uvec2(textureSize(inputTex_samp, 0));
  uvec2 baseIndex = (((WorkGroupID.xy * uvec2(params.inner.blockDim, 4u)) + (LocalInvocationID.xy * uvec2(4u, 1u))) - uvec2(filterOffset, 0u));
  {
    for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          uvec2 loadIndex = (baseIndex + uvec2(c, r));
          if ((flip.inner.value != 0u)) {
            loadIndex = loadIndex.yx;
          }
          tile[r][((4u * LocalInvocationID.x) + c)] = textureLod(inputTex_samp, ((vec2(loadIndex) + vec2(0.25f)) / vec2(dims)), 0.0f).rgb;
        }
      }
    }
  }
  barrier();
  {
    for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          uvec2 writeIndex = (baseIndex + uvec2(c, r));
          if ((flip.inner.value != 0u)) {
            writeIndex = writeIndex.yx;
          }
          uint center = ((4u * LocalInvocationID.x) + c);
          bool tint_tmp_1 = (center >= filterOffset);
          if (tint_tmp_1) {
            tint_tmp_1 = (center < (256u - filterOffset));
          }
          bool tint_tmp = (tint_tmp_1);
          if (tint_tmp) {
            tint_tmp = all(lessThan(writeIndex, dims));
          }
          if ((tint_tmp)) {
            vec3 acc = vec3(0.0f);
            {
              for(uint f = 0u; (f < params.inner.filterDim); f = (f + 1u)) {
                uint i = ((center + f) - filterOffset);
                acc = (acc + ((1.0f / float(params.inner.filterDim)) * tile[r][i]));
              }
            }
            imageStore(outputTex, ivec2(writeIndex), vec4(acc, 1.0f));
          }
        }
      }
    }
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_WorkGroupID, gl_LocalInvocationID, gl_LocalInvocationIndex);
  return;
}
