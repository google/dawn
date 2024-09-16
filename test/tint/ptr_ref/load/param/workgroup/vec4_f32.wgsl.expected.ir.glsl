#version 310 es

shared vec4 S;
vec4 func() {
  return S;
}
void tint_symbol_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    S = vec4(0.0f);
  }
  barrier();
  vec4 r = func();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
