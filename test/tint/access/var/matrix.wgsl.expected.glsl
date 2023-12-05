#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  float inner;
} s;

void tint_symbol() {
  mat3 m = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 v = m[1];
  float f = v[1];
  s.inner = f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
