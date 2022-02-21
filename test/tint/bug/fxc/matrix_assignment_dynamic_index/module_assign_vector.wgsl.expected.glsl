#version 310 es

struct Uniforms {
  uint i;
  uint j;
};

layout(binding = 4) uniform Uniforms_1 {
  uint i;
  uint j;
} uniforms;

mat2x4 m1 = mat2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
void tint_symbol() {
  m1[uniforms.i] = vec4(1.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
