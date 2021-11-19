#version 310 es
precision mediump float;



layout (binding = 1) uniform Params_1 {
  uint filterDim;
  uint blockDim;
} params;
uniform highp sampler2D inputTex;
uniform highp writeonly image2D outputTex;

layout (binding = 3) uniform Flip_1 {
  uint value;
} flip;
shared vec3 tile[4][256];

struct tint_symbol_2 {
  uvec3 LocalInvocationID;
  uint local_invocation_index;
  uvec3 WorkGroupID;
};

void tint_symbol_inner(uvec3 WorkGroupID, uvec3 LocalInvocationID, uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 1024u); idx = (idx + 64u)) {
      uint i_1 = (idx / 256u);
      uint i_2 = (idx % 256u);
      tile[i_1][i_2] = vec3(0.0f, 0.0f, 0.0f);
    }
  }
  memoryBarrierShared();
  uint filterOffset = ((params.filterDim - 1u) / 2u);
  ivec2 dims = textureSize(inputTex, 0);
  ivec2 baseIndex = (ivec2(((WorkGroupID.xy * uvec2(params.blockDim, 4u)) + (LocalInvocationID.xy * uvec2(4u, 1u)))) - ivec2(int(filterOffset), 0));
  {
    for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          ivec2 loadIndex = (baseIndex + ivec2(int(c), int(r)));
          if ((flip.value != 0u)) {
            loadIndex = loadIndex.yx;
          }
          tile[r][((4u * LocalInvocationID.x) + c)] = textureLod(inputTex, ((vec2(loadIndex) + vec2(0.25f, 0.25f)) / vec2(dims)), 0.0f).rgb;
        }
      }
    }
  }
  memoryBarrierShared();
  {
    for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          ivec2 writeIndex = (baseIndex + ivec2(int(c), int(r)));
          if ((flip.value != 0u)) {
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
            vec3 acc = vec3(0.0f, 0.0f, 0.0f);
            {
              for(uint f = 0u; (f < params.filterDim); f = (f + 1u)) {
                uint i = ((center + f) - filterOffset);
                acc = (acc + ((1.0f / float(params.filterDim)) * tile[r][i]));
              }
            }
            imageStore(outputTex, writeIndex, vec4(acc, 1.0f));
          }
        }
      }
    }
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.WorkGroupID, tint_symbol_1.LocalInvocationID, tint_symbol_1.local_invocation_index);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.LocalInvocationID = gl_LocalInvocationID;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  inputs.WorkGroupID = gl_WorkGroupID;
  tint_symbol(inputs);
}


