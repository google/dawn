struct main_inputs {
  uint3 local_id : SV_GroupThreadID;
  uint tint_local_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};


ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
RWByteAddressBuffer resultMatrix : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[1];
};
groupshared float mm_Asub[64][64];
groupshared float mm_Bsub[64][64];
float mm_readA(uint row, uint col) {
  bool v = false;
  if ((row < uniforms[0u].x)) {
    v = (col < uniforms[0u].y);
  } else {
    v = false;
  }
  if (v) {
    uint v_1 = 0u;
    firstMatrix.GetDimensions(v_1);
    float result = asfloat(firstMatrix.Load((0u + (min(((row * uniforms[0u].y) + col), ((v_1 / 4u) - 1u)) * 4u))));
    return result;
  }
  return 0.0f;
}

float mm_readB(uint row, uint col) {
  bool v_2 = false;
  if ((row < uniforms[0u].y)) {
    v_2 = (col < uniforms[0u].z);
  } else {
    v_2 = false;
  }
  if (v_2) {
    uint v_3 = 0u;
    secondMatrix.GetDimensions(v_3);
    float result = asfloat(secondMatrix.Load((0u + (min(((row * uniforms[0u].z) + col), ((v_3 / 4u) - 1u)) * 4u))));
    return result;
  }
  return 0.0f;
}

void mm_write(uint row, uint col, float value) {
  bool v_4 = false;
  if ((row < uniforms[0u].x)) {
    v_4 = (col < uniforms[0u].z);
  } else {
    v_4 = false;
  }
  if (v_4) {
    uint index = (col + (row * uniforms[0u].z));
    uint v_5 = 0u;
    resultMatrix.GetDimensions(v_5);
    resultMatrix.Store((0u + (min(index, ((v_5 / 4u) - 1u)) * 4u)), asuint(value));
  }
}

uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

void main_inner(uint3 local_id, uint3 global_id, uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 4096u)) {
        break;
      }
      mm_Asub[(v_6 / 64u)][(v_6 % 64u)] = 0.0f;
      mm_Bsub[(v_6 / 64u)][(v_6 % 64u)] = 0.0f;
      {
        idx = (idx + 256u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint tileRow = (local_id.y * 4u);
  uint tileCol = (local_id.x * 4u);
  uint globalRow = (global_id.y * 4u);
  uint globalCol = (global_id.x * 4u);
  uint numTiles = (tint_div_u32((uniforms[0u].y - 1u), 64u) + 1u);
  float acc[16] = (float[16])0;
  float ACached = 0.0f;
  float BCached[4] = (float[4])0;
  {
    uint index = 0u;
    for( ; (index < 16u); index = (index + 1u)) {
      uint v_7 = index;
      acc[v_7] = 0.0f;
    }
  }
  uint ColPerThreadA = 4u;
  uint tileColA = (local_id.x * ColPerThreadA);
  uint RowPerThreadB = 4u;
  uint tileRowB = (local_id.y * RowPerThreadB);
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    uint t = 0u;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      if ((t < numTiles)) {
      } else {
        break;
      }
      {
        uint innerRow = 0u;
        for( ; (innerRow < 4u); innerRow = (innerRow + 1u)) {
          {
            uint2 tint_loop_idx_1 = (4294967295u).xx;
            uint innerCol = 0u;
            while(true) {
              if (all((tint_loop_idx_1 == (0u).xx))) {
                break;
              }
              if ((innerCol < ColPerThreadA)) {
              } else {
                break;
              }
              uint inputRow = (tileRow + innerRow);
              uint inputCol = (tileColA + innerCol);
              mm_Asub[inputRow][min(inputCol, 63u)] = mm_readA((globalRow + innerRow), ((t * 64u) + inputCol));
              {
                uint tint_low_inc_1 = (tint_loop_idx_1.x - 1u);
                tint_loop_idx_1.x = tint_low_inc_1;
                uint tint_carry_1 = uint((tint_low_inc_1 == 4294967295u));
                tint_loop_idx_1.y = (tint_loop_idx_1.y - tint_carry_1);
                innerCol = (innerCol + 1u);
              }
            }
          }
        }
      }
      {
        uint2 tint_loop_idx_2 = (4294967295u).xx;
        uint innerRow = 0u;
        while(true) {
          if (all((tint_loop_idx_2 == (0u).xx))) {
            break;
          }
          if ((innerRow < RowPerThreadB)) {
          } else {
            break;
          }
          {
            uint innerCol = 0u;
            for( ; (innerCol < 4u); innerCol = (innerCol + 1u)) {
              uint inputRow = (tileRowB + innerRow);
              uint inputCol = (tileCol + innerCol);
              uint v_8 = innerCol;
              mm_Bsub[v_8][inputCol] = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
            }
          }
          {
            uint tint_low_inc_2 = (tint_loop_idx_2.x - 1u);
            tint_loop_idx_2.x = tint_low_inc_2;
            uint tint_carry_2 = uint((tint_low_inc_2 == 4294967295u));
            tint_loop_idx_2.y = (tint_loop_idx_2.y - tint_carry_2);
            innerRow = (innerRow + 1u);
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
      {
        uint k = 0u;
        for( ; (k < 64u); k = (k + 1u)) {
          {
            uint inner = 0u;
            for( ; (inner < 4u); inner = (inner + 1u)) {
              uint v_9 = inner;
              uint v_10 = k;
              uint v_11 = (tileCol + inner);
              BCached[v_9] = mm_Bsub[v_10][v_11];
            }
          }
          {
            uint innerRow = 0u;
            for( ; (innerRow < 4u); innerRow = (innerRow + 1u)) {
              uint v_12 = (tileRow + innerRow);
              uint v_13 = k;
              ACached = mm_Asub[v_12][v_13];
              {
                uint innerCol = 0u;
                for( ; (innerCol < 4u); innerCol = (innerCol + 1u)) {
                  uint index = ((innerRow * 4u) + innerCol);
                  float v_14 = acc[index];
                  float v_15 = ACached;
                  uint v_16 = innerCol;
                  acc[index] = (v_14 + (v_15 * BCached[v_16]));
                }
              }
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        t = (t + 1u);
      }
    }
  }
  {
    uint innerRow = 0u;
    for( ; (innerRow < 4u); innerRow = (innerRow + 1u)) {
      {
        uint innerCol = 0u;
        for( ; (innerCol < 4u); innerCol = (innerCol + 1u)) {
          uint index = ((innerRow * 4u) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[index]);
        }
      }
    }
  }
}

[numthreads(16, 16, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.local_id, inputs.global_id, inputs.tint_local_index);
}

