#version 310 es

struct Uniforms {
  uvec2 aShape;
  uvec2 bShape;
  uvec2 outShape;
};

layout(binding = 0, std430) buffer Matrix_1 {
  uint numbers[];
} firstMatrix;
layout(binding = 1, std430) buffer Matrix_2 {
  uint numbers[];
} secondMatrix;
layout(binding = 2, std430) buffer Matrix_3 {
  uint numbers[];
} resultMatrix;
layout(binding = 3) uniform Uniforms_1 {
  uvec2 aShape;
  uvec2 bShape;
  uvec2 outShape;
} uniforms;

void tint_symbol(uvec3 global_id) {
  uvec2 resultCell = uvec2(global_id.y, global_id.x);
  uint dimInner = uniforms.aShape.y;
  uint dimOutter = uniforms.outShape.y;
  uint result = 0u;
  {
    for(uint i = 0u; (i < dimInner); i = (i + 1u)) {
      uint a = (i + (resultCell.x * dimInner));
      uint b = (resultCell.y + (i * dimOutter));
      result = (result + (firstMatrix.numbers[a] * secondMatrix.numbers[b]));
    }
  }
  uint index = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.numbers[index] = result;
}

layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
void main() {
  tint_symbol(gl_GlobalInvocationID);
  return;
}
