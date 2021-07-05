ByteAddressBuffer firstMatrix : register(t0, space0);
ByteAddressBuffer secondMatrix : register(t1, space0);
RWByteAddressBuffer resultMatrix : register(u2, space0);
cbuffer cbuffer_uniforms : register(b3, space0) {
  uint4 uniforms[1];
};

float mm_readA(uint row, uint col) {
  const uint scalar_offset = (0u) / 4;
  bool tint_tmp = (row < uniforms[scalar_offset / 4][scalar_offset % 4]);
  if (tint_tmp) {
    const uint scalar_offset_1 = (4u) / 4;
    tint_tmp = (col < uniforms[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  }
  if ((tint_tmp)) {
    const uint scalar_offset_2 = (4u) / 4;
    const float result = asfloat(firstMatrix.Load((4u * ((row * uniforms[scalar_offset_2 / 4][scalar_offset_2 % 4]) + col))));
    return result;
  }
  return 0.0f;
}

float mm_readB(uint row, uint col) {
  const uint scalar_offset_3 = (4u) / 4;
  bool tint_tmp_1 = (row < uniforms[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  if (tint_tmp_1) {
    const uint scalar_offset_4 = (8u) / 4;
    tint_tmp_1 = (col < uniforms[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  }
  if ((tint_tmp_1)) {
    const uint scalar_offset_5 = (8u) / 4;
    const float result = asfloat(secondMatrix.Load((4u * ((row * uniforms[scalar_offset_5 / 4][scalar_offset_5 % 4]) + col))));
    return result;
  }
  return 0.0f;
}

void mm_write(uint row, uint col, float value) {
  const uint scalar_offset_6 = (0u) / 4;
  bool tint_tmp_2 = (row < uniforms[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  if (tint_tmp_2) {
    const uint scalar_offset_7 = (8u) / 4;
    tint_tmp_2 = (col < uniforms[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  }
  if ((tint_tmp_2)) {
    const uint scalar_offset_8 = (8u) / 4;
    const uint index = (col + (row * uniforms[scalar_offset_8 / 4][scalar_offset_8 % 4]));
    resultMatrix.Store((4u * index), asuint(value));
  }
}

static const uint RowPerThread = 4u;
static const uint ColPerThread = 4u;
static const uint TileAOuter = 64u;
static const uint TileBOuter = 64u;
static const uint TileInner = 64u;
groupshared float mm_Asub[64][64];
groupshared float mm_Bsub[64][64];

struct tint_symbol_1 {
  uint3 local_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};

[numthreads(16, 16, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint3 local_id = tint_symbol.local_id;
  const uint3 global_id = tint_symbol.global_id;
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    for(int i = 0; (i < 64); i = (i + 1)) {
      for(int i_1 = 0; (i_1 < 64); i_1 = (i_1 + 1)) {
        mm_Asub[i][i_1] = 0.0f;
      }
    }
    for(int i_2 = 0; (i_2 < 64); i_2 = (i_2 + 1)) {
      for(int i_3 = 0; (i_3 < 64); i_3 = (i_3 + 1)) {
        mm_Bsub[i_2][i_3] = 0.0f;
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  const uint tileRow = (local_id.y * RowPerThread);
  const uint tileCol = (local_id.x * ColPerThread);
  const uint globalRow = (global_id.y * RowPerThread);
  const uint globalCol = (global_id.x * ColPerThread);
  const uint scalar_offset_9 = (4u) / 4;
  const uint numTiles = (((uniforms[scalar_offset_9 / 4][scalar_offset_9 % 4] - 1u) / TileInner) + 1u);
  float acc[16] = (float[16])0;
  float ACached = 0.0f;
  float BCached[4] = (float[4])0;
  {
    uint index = 0u;
    for(; !(!((index < (RowPerThread * ColPerThread)))); index = (index + 1u)) {
      acc[index] = 0.0f;
    }
  }
  const uint ColPerThreadA = (TileInner / 16u);
  const uint tileColA = (local_id.x * ColPerThreadA);
  const uint RowPerThreadB = (TileInner / 16u);
  const uint tileRowB = (local_id.y * RowPerThreadB);
  {
    uint t = 0u;
    for(; !(!((t < numTiles))); t = (t + 1u)) {
      {
        uint innerRow = 0u;
        for(; !(!((innerRow < RowPerThread))); innerRow = (innerRow + 1u)) {
          {
            uint innerCol = 0u;
            for(; !(!((innerCol < ColPerThreadA))); innerCol = (innerCol + 1u)) {
              const uint inputRow = (tileRow + innerRow);
              const uint inputCol = (tileColA + innerCol);
              mm_Asub[inputRow][inputCol] = mm_readA((globalRow + innerRow), ((t * TileInner) + inputCol));
            }
          }
        }
      }
      {
        uint innerRow = 0u;
        for(; !(!((innerRow < RowPerThreadB))); innerRow = (innerRow + 1u)) {
          {
            uint innerCol = 0u;
            for(; !(!((innerCol < ColPerThread))); innerCol = (innerCol + 1u)) {
              const uint inputRow = (tileRowB + innerRow);
              const uint inputCol = (tileCol + innerCol);
              mm_Bsub[innerCol][inputCol] = mm_readB(((t * TileInner) + inputRow), (globalCol + innerCol));
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
      {
        uint k = 0u;
        for(; !(!((k < TileInner))); k = (k + 1u)) {
          {
            uint inner = 0u;
            for(; !(!((inner < ColPerThread))); inner = (inner + 1u)) {
              BCached[inner] = mm_Bsub[k][(tileCol + inner)];
            }
          }
          {
            uint innerRow = 0u;
            for(; !(!((innerRow < RowPerThread))); innerRow = (innerRow + 1u)) {
              ACached = mm_Asub[(tileRow + innerRow)][k];
              {
                uint innerCol = 0u;
                for(; !(!((innerCol < ColPerThread))); innerCol = (innerCol + 1u)) {
                  const uint index = ((innerRow * ColPerThread) + innerCol);
                  acc[index] = (acc[index] + (ACached * BCached[innerCol]));
                }
              }
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
    }
  }
  {
    uint innerRow = 0u;
    for(; !(!((innerRow < RowPerThread))); innerRow = (innerRow + 1u)) {
      {
        uint innerCol = 0u;
        for(; !(!((innerCol < ColPerThread))); innerCol = (innerCol + 1u)) {
          const uint index = ((innerRow * ColPerThread) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[index]);
        }
      }
    }
  }
  return;
}
