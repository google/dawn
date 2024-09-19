struct main_inputs {
  uint3 LocalInvocationID : SV_GroupThreadID;
  uint tint_local_index : SV_GroupIndex;
  uint3 WorkGroupID : SV_GroupID;
};


SamplerState samp : register(s0);
cbuffer cbuffer_params : register(b1) {
  uint4 params[1];
};
Texture2D<float4> inputTex : register(t1, space1);
RWTexture2D<float4> outputTex : register(u2, space1);
cbuffer cbuffer_flip : register(b3, space1) {
  uint4 flip[1];
};
groupshared float3 tile[4][256];
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

void main_inner(uint3 WorkGroupID, uint3 LocalInvocationID, uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 1024u)) {
        break;
      }
      tile[(v_1 / 256u)][(v_1 % 256u)] = (0.0f).xxx;
      {
        v = (v_1 + 64u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint filterOffset = tint_div_u32((params[0u].x - 1u), 2u);
  uint3 v_2 = (0u).xxx;
  inputTex.GetDimensions(uint(int(0)), v_2[0u], v_2[1u], v_2[2u]);
  uint2 dims = v_2.xy;
  uint2 v_3 = ((WorkGroupID.xy * uint2(params[0u].y, 4u)) + (LocalInvocationID.xy * uint2(4u, 1u)));
  uint2 baseIndex = (v_3 - uint2(filterOffset, 0u));
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
          uint2 loadIndex = (baseIndex + uint2(c, r));
          if ((flip[0u].x != 0u)) {
            loadIndex = loadIndex.yx;
          }
          float3 v_4 = tile[r][((4u * LocalInvocationID[0u]) + c)];
          float2 v_5 = (float2(loadIndex) + (0.25f).xx);
          float2 v_6 = (v_5 / float2(dims));
          v_4 = inputTex.SampleLevel(samp, v_6, float(0.0f)).xyz;
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
  GroupMemoryBarrierWithGroupSync();
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
          uint2 writeIndex = (baseIndex + uint2(c, r));
          if ((flip[0u].x != 0u)) {
            writeIndex = writeIndex.yx;
          }
          uint center = ((4u * LocalInvocationID[0u]) + c);
          bool v_7 = false;
          if ((center >= filterOffset)) {
            v_7 = (center < (256u - filterOffset));
          } else {
            v_7 = false;
          }
          bool v_8 = false;
          if (v_7) {
            v_8 = all((writeIndex < dims));
          } else {
            v_8 = false;
          }
          if (v_8) {
            float3 acc = (0.0f).xxx;
            {
              uint f = 0u;
              while(true) {
                if ((f < params[0u].x)) {
                } else {
                  break;
                }
                uint i = ((center + f) - filterOffset);
                float3 v_9 = acc;
                float v_10 = (1.0f / float(params[0u].x));
                acc = (v_9 + (v_10 * tile[r][i]));
                {
                  f = (f + 1u);
                }
                continue;
              }
            }
            uint2 v_11 = writeIndex;
            outputTex[v_11] = float4(acc, 1.0f);
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

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.WorkGroupID, inputs.LocalInvocationID, inputs.tint_local_index);
}

