SKIP: FAILED

#version 310 es

struct Matrix {
  uint numbers[];
};

struct Uniforms {
  uvec2 aShape;
  uvec2 bShape;
  uvec2 outShape;
};

Matrix firstMatrix;
Matrix secondMatrix;
Matrix resultMatrix;
uniform Uniforms uniforms;
layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
void main(uvec3 global_id) {
  uvec2 resultCell = uvec2(global_id[1u], global_id[0u]);
  uint dimInner = uniforms.aShape.y;
  uint dimOutter = uniforms.outShape.y;
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
error: Error parsing GLSL shader:
ERROR: 0:4: '' : array size required 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
