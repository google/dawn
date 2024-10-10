#version 310 es


struct Uniforms {
  uvec2 aShape;
  uvec2 bShape;
  uvec2 outShape;
};

layout(binding = 0, std430)
buffer Matrix_1_ssbo {
  uint numbers[];
} firstMatrix;
layout(binding = 1, std430)
buffer Matrix_2_ssbo {
  uint numbers[];
} secondMatrix;
layout(binding = 2, std430)
buffer Matrix_3_ssbo {
  uint numbers[];
} resultMatrix;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
void tint_symbol_inner(uvec3 global_id) {
  uvec2 resultCell = uvec2(global_id[1u], global_id[0u]);
  uint dimInner = v.inner.aShape.y;
  uint dimOutter = v.inner.outShape.y;
  uint result = 0u;
  {
    uint i = 0u;
    while(true) {
      if ((i < dimInner)) {
      } else {
        break;
      }
      uint a = (i + (resultCell[0u] * dimInner));
      uint b = (resultCell[1u] + (i * dimOutter));
      result = (result + (firstMatrix.numbers[a] * secondMatrix.numbers[b]));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint index = (resultCell[1u] + (resultCell[0u] * dimOutter));
  resultMatrix.numbers[index] = result;
}
layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_GlobalInvocationID);
}
