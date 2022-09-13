#version 310 es

struct OuterS {
  mat2x4 m1;
};

layout(binding = 4, std140) uniform Uniforms_ubo {
  uint i;
  uint pad;
  uint pad_1;
  uint pad_2;
} uniforms;

void tint_symbol() {
  OuterS s1 = OuterS(mat2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  s1.m1[uniforms.i] = vec4(1.0f);
  s1.m1[uniforms.i][uniforms.i] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
