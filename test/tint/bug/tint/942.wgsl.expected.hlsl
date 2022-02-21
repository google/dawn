SamplerState samp : register(s0, space0);
cbuffer cbuffer_params : register(b1, space0) {
  uint4 params[1];
};
Texture2D<float4> inputTex : register(t1, space1);
RWTexture2D<float4> outputTex : register(u2, space1);

cbuffer cbuffer_flip : register(b3, space1) {
  uint4 flip[1];
};
groupshared float3 tile[4][256];

struct tint_symbol_1 {
  uint3 LocalInvocationID : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 WorkGroupID : SV_GroupID;
};

void main_inner(uint3 WorkGroupID, uint3 LocalInvocationID, uint local_invocation_index) {
  {
    [loop] for(uint idx = local_invocation_index; (idx < 1024u); idx = (idx + 64u)) {
      const uint i_1 = (idx / 256u);
      const uint i_2 = (idx % 256u);
      tile[i_1][i_2] = float3(0.0f, 0.0f, 0.0f);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  const uint filterOffset = ((params[0].x - 1u) / 2u);
  int3 tint_tmp;
  inputTex.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  const int2 dims = tint_tmp.xy;
  const int2 baseIndex = (int2(((WorkGroupID.xy * uint2(params[0].y, 4u)) + (LocalInvocationID.xy * uint2(4u, 1u)))) - int2(int(filterOffset), 0));
  {
    [loop] for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        [loop] for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          int2 loadIndex = (baseIndex + int2(int(c), int(r)));
          if ((flip[0].x != 0u)) {
            loadIndex = loadIndex.yx;
          }
          tile[r][((4u * LocalInvocationID.x) + c)] = inputTex.SampleLevel(samp, ((float2(loadIndex) + float2(0.25f, 0.25f)) / float2(dims)), 0.0f).rgb;
        }
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  {
    [loop] for(uint r = 0u; (r < 4u); r = (r + 1u)) {
      {
        [loop] for(uint c = 0u; (c < 4u); c = (c + 1u)) {
          int2 writeIndex = (baseIndex + int2(int(c), int(r)));
          if ((flip[0].x != 0u)) {
            writeIndex = writeIndex.yx;
          }
          const uint center = ((4u * LocalInvocationID.x) + c);
          bool tint_tmp_2 = (center >= filterOffset);
          if (tint_tmp_2) {
            tint_tmp_2 = (center < (256u - filterOffset));
          }
          bool tint_tmp_1 = (tint_tmp_2);
          if (tint_tmp_1) {
            tint_tmp_1 = all((writeIndex < dims));
          }
          if ((tint_tmp_1)) {
            float3 acc = float3(0.0f, 0.0f, 0.0f);
            {
              [loop] for(uint f = 0u; (f < params[0].x); f = (f + 1u)) {
                uint i = ((center + f) - filterOffset);
                acc = (acc + ((1.0f / float(params[0].x)) * tile[r][i]));
              }
            }
            outputTex[writeIndex] = float4(acc, 1.0f);
          }
        }
      }
    }
  }
}

[numthreads(64, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.WorkGroupID, tint_symbol.LocalInvocationID, tint_symbol.local_invocation_index);
  return;
}
