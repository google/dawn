#version 310 es
precision mediump float;


layout (binding = 0) buffer Matrix_1 {
  float numbers[];
} firstMatrix;
layout (binding = 1) buffer Matrix_2 {
  float numbers[];
} secondMatrix;
layout (binding = 2) buffer Matrix_3 {
  float numbers[];
} resultMatrix;
layout (binding = 3) uniform Uniforms_1 {
  uint dimAOuter;
  uint dimInner;
  uint dimBOuter;
} uniforms;

float mm_readA(uint row, uint col) {
  bool tint_tmp = (row < uniforms.dimAOuter);
  if (tint_tmp) {
    tint_tmp = (col < uniforms.dimInner);
  }
  if ((tint_tmp)) {
    float result = firstMatrix.numbers[((row * uniforms.dimInner) + col)];
    return result;
  }
  return 0.0f;
}

float mm_readB(uint row, uint col) {
  bool tint_tmp_1 = (row < uniforms.dimInner);
  if (tint_tmp_1) {
    tint_tmp_1 = (col < uniforms.dimBOuter);
  }
  if ((tint_tmp_1)) {
    float result = secondMatrix.numbers[((row * uniforms.dimBOuter) + col)];
    return result;
  }
  return 0.0f;
}

void mm_write(uint row, uint col, float value) {
  bool tint_tmp_2 = (row < uniforms.dimAOuter);
  if (tint_tmp_2) {
    tint_tmp_2 = (col < uniforms.dimBOuter);
  }
  if ((tint_tmp_2)) {
    uint index = (col + (row * uniforms.dimBOuter));
    resultMatrix.numbers[index] = value;
  }
}

const uint RowPerThread = 4u;
const uint ColPerThread = 4u;
const uint TileInner = 64u;
shared float mm_Asub[64][64];
shared float mm_Bsub[64][64];

struct tint_symbol_2 {
  uvec3 local_id;
  uint local_invocation_index;
  uvec3 global_id;
};

void tint_symbol_inner(uvec3 local_id, uvec3 global_id, uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4096u); idx = (idx + 256u)) {
      uint i = (idx / 64u);
      uint i_1 = (idx % 64u);
      mm_Asub[i][i_1] = 0.0f;
      mm_Bsub[i][i_1] = 0.0f;
    }
  }
  memoryBarrierShared();
  uint tileRow = (local_id.y * RowPerThread);
  uint tileCol = (local_id.x * ColPerThread);
  uint globalRow = (global_id.y * RowPerThread);
  uint globalCol = (global_id.x * ColPerThread);
  uint numTiles = (((uniforms.dimInner - 1u) / TileInner) + 1u);
  float acc[16] = float[16](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float ACached = 0.0f;
  float BCached[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    for(uint index = 0u; (index < (RowPerThread * ColPerThread)); index = (index + 1u)) {
      acc[index] = 0.0f;
    }
  }
  uint ColPerThreadA = (TileInner / 16u);
  uint tileColA = (local_id.x * ColPerThreadA);
  uint RowPerThreadB = (TileInner / 16u);
  uint tileRowB = (local_id.y * RowPerThreadB);
  {
    for(uint t = 0u; (t < numTiles); t = (t + 1u)) {
      {
        for(uint innerRow = 0u; (innerRow < RowPerThread); innerRow = (innerRow + 1u)) {
          {
            for(uint innerCol = 0u; (innerCol < ColPerThreadA); innerCol = (innerCol + 1u)) {
              uint inputRow = (tileRow + innerRow);
              uint inputCol = (tileColA + innerCol);
              mm_Asub[inputRow][inputCol] = mm_readA((globalRow + innerRow), ((t * TileInner) + inputCol));
            }
          }
        }
      }
      {
        for(uint innerRow = 0u; (innerRow < RowPerThreadB); innerRow = (innerRow + 1u)) {
          {
            for(uint innerCol = 0u; (innerCol < ColPerThread); innerCol = (innerCol + 1u)) {
              uint inputRow = (tileRowB + innerRow);
              uint inputCol = (tileCol + innerCol);
              mm_Bsub[innerCol][inputCol] = mm_readB(((t * TileInner) + inputRow), (globalCol + innerCol));
            }
          }
        }
      }
      memoryBarrierShared();
      {
        for(uint k = 0u; (k < TileInner); k = (k + 1u)) {
          {
            for(uint inner = 0u; (inner < ColPerThread); inner = (inner + 1u)) {
              BCached[inner] = mm_Bsub[k][(tileCol + inner)];
            }
          }
          {
            for(uint innerRow = 0u; (innerRow < RowPerThread); innerRow = (innerRow + 1u)) {
              ACached = mm_Asub[(tileRow + innerRow)][k];
              {
                for(uint innerCol = 0u; (innerCol < ColPerThread); innerCol = (innerCol + 1u)) {
                  uint index = ((innerRow * ColPerThread) + innerCol);
                  acc[index] = (acc[index] + (ACached * BCached[innerCol]));
                }
              }
            }
          }
        }
      }
      memoryBarrierShared();
    }
  }
  {
    for(uint innerRow = 0u; (innerRow < RowPerThread); innerRow = (innerRow + 1u)) {
      {
        for(uint innerCol = 0u; (innerCol < ColPerThread); innerCol = (innerCol + 1u)) {
          uint index = ((innerRow * ColPerThread) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[index]);
        }
      }
    }
  }
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.local_id, tint_symbol_1.global_id, tint_symbol_1.local_invocation_index);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.local_id = gl_LocalInvocationID;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  inputs.global_id = gl_GlobalInvocationID;
  tint_symbol(inputs);
}


