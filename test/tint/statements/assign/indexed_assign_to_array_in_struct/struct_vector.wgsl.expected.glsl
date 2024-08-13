#version 310 es

struct Uniforms {
  uint i;
};

struct OuterS {
  vec3 v1;
};

layout(binding = 4, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

void tint_symbol() {
  OuterS s1 = OuterS(vec3(0.0f, 0.0f, 0.0f));
  s1.v1[uniforms.inner.i] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
