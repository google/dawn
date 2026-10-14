#version 310 es

layout(binding = 1, std430)
buffer Matrix_1_ssbo {
  float numbers[];
} firstMatrix;
layout(binding = 2, std430)
buffer Matrix_2_ssbo {
  float numbers[];
} secondMatrix;
layout(binding = 3, std430)
buffer Matrix_3_ssbo {
  float numbers[];
} resultMatrix;
layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v;
shared float mm_Asub[64][64];
shared float mm_Bsub[64][64];
float mm_readA(uint row, uint col) {
  uvec4 v_1 = v.inner[0u];
  bool v_2 = false;
  if ((row < v_1.x)) {
    uvec4 v_3 = v.inner[0u];
    v_2 = (col < v_3.y);
  } else {
    v_2 = false;
  }
  if (v_2) {
    uvec4 v_4 = v.inner[0u];
    uint v_5 = min(((row * v_4.y) + col), (uint(firstMatrix.numbers.length()) - 1u));
    float result = firstMatrix.numbers[v_5];
    return result;
  }
  return 0.0f;
}
float mm_readB(uint row, uint col) {
  uvec4 v_6 = v.inner[0u];
  bool v_7 = false;
  if ((row < v_6.y)) {
    uvec4 v_8 = v.inner[0u];
    v_7 = (col < v_8.z);
  } else {
    v_7 = false;
  }
  if (v_7) {
    uvec4 v_9 = v.inner[0u];
    uint v_10 = min(((row * v_9.z) + col), (uint(secondMatrix.numbers.length()) - 1u));
    float result = secondMatrix.numbers[v_10];
    return result;
  }
  return 0.0f;
}
void mm_write(uint row, uint col, float value) {
  uvec4 v_11 = v.inner[0u];
  bool v_12 = false;
  if ((row < v_11.x)) {
    uvec4 v_13 = v.inner[0u];
    v_12 = (col < v_13.z);
  } else {
    v_12 = false;
  }
  if (v_12) {
    uvec4 v_14 = v.inner[0u];
    uint index = (col + (row * v_14.z));
    uint v_15 = min(index, (uint(resultMatrix.numbers.length()) - 1u));
    resultMatrix.numbers[v_15] = value;
  }
}
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void main_inner(uvec3 local_id, uvec3 global_id, uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_16 = idx;
      if ((v_16 >= 4096u)) {
        break;
      }
      mm_Asub[(v_16 / 64u)][(v_16 % 64u)] = 0.0f;
      mm_Bsub[(v_16 / 64u)][(v_16 % 64u)] = 0.0f;
      {
        idx = (idx + 256u);
      }
    }
  }
  barrier();
  uint tileRow = (local_id.y * 4u);
  uint tileCol = (local_id.x * 4u);
  uint globalRow = (global_id.y * 4u);
  uint globalCol = (global_id.x * 4u);
  uvec4 v_17 = v.inner[0u];
  uint numTiles = (tint_div_u32((v_17.y - 1u), 64u) + 1u);
  float acc[16] = float[16](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float ACached = 0.0f;
  float BCached[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    uint index = 0u;
    while(true) {
      if ((index < 16u)) {
      } else {
        break;
      }
      uint v_18 = index;
      acc[v_18] = 0.0f;
      {
        index = (index + 1u);
      }
    }
  }
  uint ColPerThreadA = 4u;
  uint tileColA = (local_id.x * ColPerThreadA);
  uint RowPerThreadB = 4u;
  uint tileRowB = (local_id.y * RowPerThreadB);
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    uint t = 0u;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((t < numTiles)) {
      } else {
        break;
      }
      {
        uint innerRow = 0u;
        while(true) {
          if ((innerRow < 4u)) {
          } else {
            break;
          }
          {
            uvec2 tint_loop_idx_1 = uvec2(4294967295u);
            uint innerCol = 0u;
            while(true) {
              if (all(equal(tint_loop_idx_1, uvec2(0u)))) {
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
          {
            innerRow = (innerRow + 1u);
          }
        }
      }
      {
        uvec2 tint_loop_idx_2 = uvec2(4294967295u);
        uint innerRow = 0u;
        while(true) {
          if (all(equal(tint_loop_idx_2, uvec2(0u)))) {
            break;
          }
          if ((innerRow < RowPerThreadB)) {
          } else {
            break;
          }
          {
            uint innerCol = 0u;
            while(true) {
              if ((innerCol < 4u)) {
              } else {
                break;
              }
              uint inputRow = (tileRowB + innerRow);
              uint inputCol = (tileCol + innerCol);
              uint v_19 = innerCol;
              mm_Bsub[v_19][inputCol] = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
              {
                innerCol = (innerCol + 1u);
              }
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
      barrier();
      {
        uint k = 0u;
        while(true) {
          if ((k < 64u)) {
          } else {
            break;
          }
          {
            uint inner = 0u;
            while(true) {
              if ((inner < 4u)) {
              } else {
                break;
              }
              uint v_20 = inner;
              uint v_21 = k;
              uint v_22 = (tileCol + inner);
              BCached[v_20] = mm_Bsub[v_21][v_22];
              {
                inner = (inner + 1u);
              }
            }
          }
          {
            uint innerRow = 0u;
            while(true) {
              if ((innerRow < 4u)) {
              } else {
                break;
              }
              uint v_23 = (tileRow + innerRow);
              uint v_24 = k;
              ACached = mm_Asub[v_23][v_24];
              {
                uint innerCol = 0u;
                while(true) {
                  if ((innerCol < 4u)) {
                  } else {
                    break;
                  }
                  uint index = ((innerRow * 4u) + innerCol);
                  float v_25 = acc[index];
                  float v_26 = ACached;
                  uint v_27 = innerCol;
                  acc[index] = (v_25 + (v_26 * BCached[v_27]));
                  {
                    innerCol = (innerCol + 1u);
                  }
                }
              }
              {
                innerRow = (innerRow + 1u);
              }
            }
          }
          {
            k = (k + 1u);
          }
        }
      }
      barrier();
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
    while(true) {
      if ((innerRow < 4u)) {
      } else {
        break;
      }
      {
        uint innerCol = 0u;
        while(true) {
          if ((innerCol < 4u)) {
          } else {
            break;
          }
          uint index = ((innerRow * 4u) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[index]);
          {
            innerCol = (innerCol + 1u);
          }
        }
      }
      {
        innerRow = (innerRow + 1u);
      }
    }
  }
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationID, gl_GlobalInvocationID, gl_LocalInvocationIndex);
}
