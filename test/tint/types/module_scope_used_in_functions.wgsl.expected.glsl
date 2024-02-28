#version 310 es

shared float w;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    w = 0.0f;
  }
  barrier();
}

float p = 0.0f;
layout(binding = 1, std430) buffer uniforms_block_ssbo {
  vec2 inner;
} uniforms;

layout(binding = 0, std430) buffer storages_block_ssbo {
  float inner[];
} storages;

void no_uses() {
}

void zoo() {
  p = (p * 2.0f);
}

void bar(float a, float b) {
  p = a;
  w = b;
  storages.inner[0] = uniforms.inner.x;
  zoo();
}

void foo(float a) {
  float b = 2.0f;
  bar(a, b);
  no_uses();
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  foo(1.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
