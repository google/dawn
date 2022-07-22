void set_vector_float2x4(inout float2x4 mat, int col, float4 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
  }
}

void set_scalar_float2x4(inout float2x4 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : mat[0];
      break;
    case 1:
      mat[1] = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : mat[1];
      break;
  }
}

struct OuterS {
  float2x4 m1;
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  set_vector_float2x4(s1.m1, uniforms[0].x, (1.0f).xxxx);
  set_scalar_float2x4(s1.m1, uniforms[0].x, uniforms[0].x, 1.0f);
  return;
}
