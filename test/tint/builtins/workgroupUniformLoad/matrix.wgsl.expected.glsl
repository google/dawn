#version 310 es

shared mat3 v;
mat3 foo() {
  barrier();
  mat3 v_1 = v;
  barrier();
  return v_1;
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  }
  barrier();
  foo();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
