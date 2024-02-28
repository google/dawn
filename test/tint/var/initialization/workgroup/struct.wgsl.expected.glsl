#version 310 es

struct S {
  int a;
  float b;
};

shared S v;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    S tint_symbol_1 = S(0, 0.0f);
    v = tint_symbol_1;
  }
  barrier();
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
