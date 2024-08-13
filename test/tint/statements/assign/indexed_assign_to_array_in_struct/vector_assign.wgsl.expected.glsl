#version 310 es

struct Uniforms {
  uint i;
};

struct OuterS {
  uint a1[8];
};

layout(binding = 4, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

uint f(uint i) {
  return (i + 1u);
}

void tint_symbol() {
  OuterS s1 = OuterS(uint[8](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  vec3 v = vec3(0.0f, 0.0f, 0.0f);
  v[s1.a1[uniforms.inner.i]] = 1.0f;
  uint tint_symbol_1 = f(s1.a1[uniforms.inner.i]);
  v[tint_symbol_1] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
