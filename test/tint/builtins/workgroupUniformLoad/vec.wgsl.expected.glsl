#version 310 es

shared vec4 v;
vec4 foo() {
  barrier();
  vec4 v_1 = v;
  barrier();
  return v_1;
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v = vec4(0.0f);
  }
  barrier();
  foo();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
