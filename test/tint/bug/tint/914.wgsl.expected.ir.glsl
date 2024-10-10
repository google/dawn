#version 310 es


struct Uniforms {
  uint dimAOuter;
  uint dimInner;
  uint dimBOuter;
};

layout(binding = 0, std430)
buffer Matrix_1_ssbo {
  float numbers[];
} firstMatrix;
layout(binding = 1, std430)
buffer Matrix_2_ssbo {
  float numbers[];
} secondMatrix;
layout(binding = 2, std430)
buffer Matrix_3_ssbo {
  float numbers[];
} resultMatrix;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
shared float mm_Asub[64][64];
shared float mm_Bsub[64][64];
float mm_readA(uint row, uint col) {
  bool v_1 = false;
  if ((row < v.inner.dimAOuter)) {
    v_1 = (col < v.inner.dimInner);
  } else {
    v_1 = false;
  }
  if (v_1) {
    uint v_2 = ((row * v.inner.dimInner) + col);
    float result = firstMatrix.numbers[v_2];
    return result;
  }
  return 0.0f;
}
float mm_readB(uint row, uint col) {
  bool v_3 = false;
  if ((row < v.inner.dimInner)) {
    v_3 = (col < v.inner.dimBOuter);
  } else {
    v_3 = false;
  }
  if (v_3) {
    uint v_4 = ((row * v.inner.dimBOuter) + col);
    float result = secondMatrix.numbers[v_4];
    return result;
  }
  return 0.0f;
}
void mm_write(uint row, uint col, float value) {
  bool v_5 = false;
  if ((row < v.inner.dimAOuter)) {
    v_5 = (col < v.inner.dimBOuter);
  } else {
    v_5 = false;
  }
  if (v_5) {
    uint index = (col + (row * v.inner.dimBOuter));
    resultMatrix.numbers[index] = value;
  }
}
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void tint_symbol_inner(uvec3 local_id, uvec3 global_id, uint tint_local_index) {
  {
    uint v_6 = 0u;
    v_6 = tint_local_index;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4096u)) {
        break;
      }
      mm_Asub[(v_7 / 64u)][(v_7 % 64u)] = 0.0f;
      mm_Bsub[(v_7 / 64u)][(v_7 % 64u)] = 0.0f;
      {
        v_6 = (v_7 + 256u);
      }
      continue;
    }
  }
  barrier();
  uint tileRow = (local_id[1u] * 4u);
  uint tileCol = (local_id[0u] * 4u);
  uint globalRow = (global_id[1u] * 4u);
  uint globalCol = (global_id[0u] * 4u);
  uint numTiles = (tint_div_u32((v.inner.dimInner - 1u), 64u) + 1u);
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
      uint v_8 = index;
      acc[v_8] = 0.0f;
      {
        index = (index + 1u);
      }
      continue;
    }
  }
  uint ColPerThreadA = 4u;
  uint tileColA = (local_id[0u] * ColPerThreadA);
  uint RowPerThreadB = 4u;
  uint tileRowB = (local_id[1u] * RowPerThreadB);
  {
    uint t = 0u;
    while(true) {
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
            uint innerCol = 0u;
            while(true) {
              if ((innerCol < ColPerThreadA)) {
              } else {
                break;
              }
              uint inputRow = (tileRow + innerRow);
              uint inputCol = (tileColA + innerCol);
              mm_Asub[inputRow][inputCol] = mm_readA((globalRow + innerRow), ((t * 64u) + inputCol));
              {
                innerCol = (innerCol + 1u);
              }
              continue;
            }
          }
          {
            innerRow = (innerRow + 1u);
          }
          continue;
        }
      }
      {
        uint innerRow = 0u;
        while(true) {
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
              uint v_9 = innerCol;
              mm_Bsub[v_9][inputCol] = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
              {
                innerCol = (innerCol + 1u);
              }
              continue;
            }
          }
          {
            innerRow = (innerRow + 1u);
          }
          continue;
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
              uint v_10 = inner;
              uint v_11 = k;
              uint v_12 = (tileCol + inner);
              BCached[v_10] = mm_Bsub[v_11][v_12];
              {
                inner = (inner + 1u);
              }
              continue;
            }
          }
          {
            uint innerRow = 0u;
            while(true) {
              if ((innerRow < 4u)) {
              } else {
                break;
              }
              uint v_13 = (tileRow + innerRow);
              uint v_14 = k;
              ACached = mm_Asub[v_13][v_14];
              {
                uint innerCol = 0u;
                while(true) {
                  if ((innerCol < 4u)) {
                  } else {
                    break;
                  }
                  uint index = ((innerRow * 4u) + innerCol);
                  float v_15 = acc[index];
                  float v_16 = ACached;
                  uint v_17 = innerCol;
                  acc[index] = (v_15 + (v_16 * BCached[v_17]));
                  {
                    innerCol = (innerCol + 1u);
                  }
                  continue;
                }
              }
              {
                innerRow = (innerRow + 1u);
              }
              continue;
            }
          }
          {
            k = (k + 1u);
          }
          continue;
        }
      }
      barrier();
      {
        t = (t + 1u);
      }
      continue;
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
          continue;
        }
      }
      {
        innerRow = (innerRow + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationID, gl_GlobalInvocationID, gl_LocalInvocationIndex);
}
