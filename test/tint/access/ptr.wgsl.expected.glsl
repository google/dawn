#version 310 es

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

shared int g1;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    atomicExchange(g1, 0);
  }
  barrier();
}

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

struct S {
  int a;
  int b;
};

int accept_value(int val) {
  return val;
}

int accept_ptr_deref_call_func(inout int val) {
  int tint_symbol_3 = val;
  int tint_symbol_4 = accept_value(val);
  return (tint_symbol_3 + tint_symbol_4);
}

int accept_ptr_deref_pass_through(inout int val) {
  int tint_symbol_1 = val;
  int tint_symbol_2 = accept_ptr_deref_call_func(val);
  return (tint_symbol_1 + tint_symbol_2);
}

int accept_ptr_to_struct_and_access(inout S val) {
  return (val.a + val.b);
}

int accept_ptr_to_struct_access_pass_ptr(inout S val) {
  val.a = 2;
  return val.a;
}

int accept_ptr_vec_access_elements(inout vec3 v1) {
  v1.x = cross(v1, v1).x;
  return tint_ftoi(v1.x);
}

int call_builtin_with_mod_scope_ptr() {
  return atomicOr(g1, 0);
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int v1 = 0;
  S v2 = S(0, 0);
  vec3 v4 = vec3(0.0f);
  int t1 = atomicOr(g1, 0);
  int tint_symbol_5 = accept_ptr_deref_pass_through(v1);
  int tint_symbol_6 = accept_ptr_to_struct_and_access(v2);
  int tint_symbol_7 = accept_ptr_to_struct_and_access(v2);
  int tint_symbol_8 = accept_ptr_vec_access_elements(v4);
  int tint_symbol_9 = accept_ptr_to_struct_access_pass_ptr(v2);
  int tint_symbol_10 = call_builtin_with_mod_scope_ptr();
  s.inner = ((((((tint_symbol_5 + tint_symbol_6) + tint_symbol_7) + tint_symbol_8) + tint_symbol_9) + tint_symbol_10) + t1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
