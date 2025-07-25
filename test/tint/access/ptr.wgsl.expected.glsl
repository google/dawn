#version 310 es


struct S {
  int a;
  int b;
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
shared int g1;
int accept_value(int val) {
  return val;
}
int accept_ptr_deref_call_func(inout int val) {
  int v_1 = val;
  int v_2 = accept_value(val);
  uint v_3 = uint(v_1);
  return int((v_3 + uint(v_2)));
}
int accept_ptr_deref_pass_through(inout int val) {
  int v_4 = val;
  int v_5 = accept_ptr_deref_call_func(val);
  uint v_6 = uint(v_4);
  return int((v_6 + uint(v_5)));
}
int accept_ptr_to_struct_and_access(inout S val) {
  int v_7 = val.b;
  uint v_8 = uint(val.a);
  return int((v_8 + uint(v_7)));
}
int accept_ptr_to_struct_access_pass_ptr(inout S val) {
  val.a = 2;
  return val.a;
}
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
int accept_ptr_vec_access_elements(inout vec3 v1) {
  v1.x = cross(v1, v1).x;
  return tint_f32_to_i32(v1.x);
}
int call_builtin_with_mod_scope_ptr() {
  return atomicOr(g1, 0);
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(g1, 0);
  }
  barrier();
  int v1 = 0;
  S v2 = S(0, 0);
  vec3 v4 = vec3(0.0f);
  int t1 = atomicOr(g1, 0);
  int v_9 = accept_ptr_deref_pass_through(v1);
  int v_10 = accept_ptr_to_struct_and_access(v2);
  uint v_11 = uint(v_9);
  int v_12 = int((v_11 + uint(v_10)));
  int v_13 = accept_ptr_to_struct_and_access(v2);
  uint v_14 = uint(v_12);
  int v_15 = int((v_14 + uint(v_13)));
  int v_16 = accept_ptr_vec_access_elements(v4);
  uint v_17 = uint(v_15);
  int v_18 = int((v_17 + uint(v_16)));
  int v_19 = accept_ptr_to_struct_access_pass_ptr(v2);
  uint v_20 = uint(v_18);
  int v_21 = int((v_20 + uint(v_19)));
  int v_22 = call_builtin_with_mod_scope_ptr();
  uint v_23 = uint(v_21);
  uint v_24 = uint(int((v_23 + uint(v_22))));
  v.inner = int((v_24 + uint(t1)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
