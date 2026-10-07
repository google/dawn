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
  return (lhs / select((rhs == 0u), 1u, rhs));
}

void main_inner(uint3 WorkGroupID, uint3 LocalInvocationID, uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 1024u)) {
        break;
      }
      tile[(v / 256u)][(v % 256u)] = (0.0f).xxx;
      {
        idx = (idx + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint filterOffset = tint_div_u32((params[0u].x - 1u), 2u);
  uint3 v_1 = (0u).xxx;
  inputTex.GetDimensions(0u, v_1.x, v_1.y, v_1.z);
  uint2 dims = v_1.xy;
  uint2 v_2 = ((WorkGroupID.xy * uint2(params[0u].y, 4u)) + (LocalInvocationID.xy * uint2(4u, 1u)));
  uint2 baseIndex = (v_2 - uint2(filterOffset, 0u));
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
          uint v_3 = r;
          uint v_4 = ((4u * LocalInvocationID.x) + c);
          float2 v_5 = (float2(loadIndex) + (0.25f).xx);
          tile[v_3][v_4] = inputTex.SampleLevel(samp, (v_5 / float2(dims)), 0.0f).xyz;
          {
            c = (c + 1u);
          }
        }
      }
      {
        r = (r + 1u);
      }
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
          uint center = ((4u * LocalInvocationID.x) + c);
          bool v_6 = false;
          if ((center >= filterOffset)) {
            v_6 = (center < (256u - filterOffset));
          } else {
            v_6 = false;
          }
          bool v_7 = false;
          if (v_6) {
            v_7 = all((writeIndex < dims));
          } else {
            v_7 = false;
          }
          if (v_7) {
            float3 acc = (0.0f).xxx;
            {
              uint2 tint_loop_idx = (4294967295u).xx;
              uint f = 0u;
              while(true) {
                if (all((tint_loop_idx == (0u).xx))) {
                  break;
                }
                if ((f < params[0u].x)) {
                } else {
                  break;
                }
                uint i = ((center + f) - filterOffset);
                float3 v_8 = acc;
                float v_9 = (1.0f / float(params[0u].x));
                uint v_10 = r;
                uint v_11 = min(i, 255u);
                acc = (v_8 + (v_9 * tile[v_10][v_11]));
                {
                  uint tint_low_inc = (tint_loop_idx.x - 1u);
                  tint_loop_idx.x = tint_low_inc;
                  uint tint_carry = uint((tint_low_inc == 4294967295u));
                  tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
                  f = (f + 1u);
                }
              }
            }
            uint2 v_12 = writeIndex;
            outputTex[v_12] = float4(acc, 1.0f);
          }
          {
            c = (c + 1u);
          }
        }
      }
      {
        r = (r + 1u);
      }
    }
  }
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.WorkGroupID, inputs.LocalInvocationID, inputs.tint_local_index);
}

