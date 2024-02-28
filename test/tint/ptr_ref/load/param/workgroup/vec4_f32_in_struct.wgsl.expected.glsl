#version 310 es

struct str {
  vec4 i;
};

shared str S;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    str tint_symbol_1 = str(vec4(0.0f));
    S = tint_symbol_1;
  }
  barrier();
}

vec4 func_S_i() {
  return S.i;
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  vec4 r = func_S_i();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
