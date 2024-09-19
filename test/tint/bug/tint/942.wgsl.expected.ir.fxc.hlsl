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
  Texture2D<float4> v_2 = inputTex;
  uint3 v_3 = (0u).xxx;
  v_2.GetDimensions(uint(int(0)), v_3[0u], v_3[1u], v_3[2u]);
  uint2 dims = v_3.xy;
  uint2 v_4 = ((WorkGroupID.xy * uint2(params[0u].y, 4u)) + (LocalInvocationID.xy * uint2(4u, 1u)));
  uint2 baseIndex = (v_4 - uint2(filterOffset, 0u));
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
          float3 v_5 = tile[r][((4u * LocalInvocationID[0u]) + c)];
          Texture2D<float4> v_6 = inputTex;
          SamplerState v_7 = samp;
          float2 v_8 = (float2(loadIndex) + (0.25f).xx);
          float2 v_9 = (v_8 / float2(dims));
          v_5 = v_6.SampleLevel(v_7, v_9, float(0.0f)).xyz;
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
          bool v_10 = false;
          if ((center >= filterOffset)) {
            v_10 = (center < (256u - filterOffset));
          } else {
            v_10 = false;
          }
          bool v_11 = false;
          if (v_10) {
            v_11 = all((writeIndex < dims));
          } else {
            v_11 = false;
          }
          if (v_11) {
            float3 acc = (0.0f).xxx;
            {
              uint f = 0u;
              while(true) {
                if ((f < params[0u].x)) {
                } else {
                  break;
                }
                uint i = ((center + f) - filterOffset);
                float3 v_12 = acc;
                float v_13 = (1.0f / float(params[0u].x));
                acc = (v_12 + (v_13 * tile[r][i]));
                {
                  f = (f + 1u);
                }
                continue;
              }
            }
            RWTexture2D<float4> v_14 = outputTex;
            uint2 v_15 = writeIndex;
            v_14[v_15] = float4(acc, 1.0f);
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

