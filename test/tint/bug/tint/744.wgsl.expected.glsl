#version 310 es

layout(binding = 2, std430)
buffer Matrix_1_ssbo {
  uint numbers[];
} firstMatrix;
layout(binding = 3, std430)
buffer Matrix_2_ssbo {
  uint numbers[];
} secondMatrix;
layout(binding = 0, std430)
buffer Matrix_3_ssbo {
  uint numbers[];
} resultMatrix;
layout(binding = 1, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[2];
} v;
void main_inner(uvec3 global_id) {
  uvec2 resultCell = uvec2(global_id.y, global_id.x);
  uvec4 v_1 = v.inner[0u];
  uint dimInner = v_1.y;
  uvec4 v_2 = v.inner[1u];
  uint dimOutter = v_2.y;
  uint result = 0u;
  {
    uint i = 0u;
    while(true) {
      if ((i < dimInner)) {
      } else {
        break;
      }
      uint a = (i + (resultCell.x * dimInner));
      uint b = (resultCell.y + (i * dimOutter));
      uint v_3 = result;
      uint v_4 = min(a, (uint(firstMatrix.numbers.length()) - 1u));
      uint v_5 = firstMatrix.numbers[v_4];
      uint v_6 = min(b, (uint(secondMatrix.numbers.length()) - 1u));
      result = (v_3 + (v_5 * secondMatrix.numbers[v_6]));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint index = (resultCell.y + (resultCell.x * dimOutter));
  uint v_7 = min(index, (uint(resultMatrix.numbers.length()) - 1u));
  resultMatrix.numbers[v_7] = result;
}
layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
