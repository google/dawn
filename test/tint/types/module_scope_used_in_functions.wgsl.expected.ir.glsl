#version 310 es

float p = 0.0f;
shared float w;
layout(binding = 1, std430)
buffer tint_symbol_2_1_ssbo {
  vec2 tint_symbol_1;
} v;
layout(binding = 0, std430)
buffer tint_symbol_4_1_ssbo {
  float tint_symbol_3[];
} v_1;
void no_uses() {
}
void zoo() {
  p = (p * 2.0f);
}
void bar(float a, float b) {
  p = a;
  w = b;
  v_1.tint_symbol_3[0] = v.tint_symbol_1.x;
  zoo();
}
void foo(float a) {
  float b = 2.0f;
  bar(a, b);
  no_uses();
}
void tint_symbol_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    w = 0.0f;
  }
  barrier();
  foo(1.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
