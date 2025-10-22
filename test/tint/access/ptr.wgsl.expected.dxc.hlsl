struct S {
  int a;
  int b;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer s : register(u0);
groupshared int g1;
int accept_value(int val) {
  return val;
}

int accept_ptr_deref_call_func(inout int val) {
  int v = val;
  return asint((asuint(v) + asuint(accept_value(val))));
}

int accept_ptr_deref_pass_through(inout int val) {
  int v_1 = val;
  return asint((asuint(v_1) + asuint(accept_ptr_deref_call_func(val))));
}

int accept_ptr_to_struct_and_access(inout S val) {
  return asint((asuint(val.a) + asuint(val.b)));
}

int accept_ptr_to_struct_access_pass_ptr(inout S val) {
  val.a = int(2);
  return val.a;
}

int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

int accept_ptr_vec_access_elements(inout float3 v1) {
  v1.x = cross(v1, v1).x;
  return tint_f32_to_i32(v1.x);
}

int call_builtin_with_mod_scope_ptr() {
  int v_2 = int(0);
  InterlockedOr(g1, int(0), v_2);
  return v_2;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v_3 = int(0);
    InterlockedExchange(g1, int(0), v_3);
  }
  GroupMemoryBarrierWithGroupSync();
  int v1 = int(0);
  S v2 = (S)0;
  float3 v4 = (0.0f).xxx;
  int v_4 = int(0);
  InterlockedOr(g1, int(0), v_4);
  int t1 = v_4;
  int v_5 = accept_ptr_deref_pass_through(v1);
  int v_6 = asint((asuint(v_5) + asuint(accept_ptr_to_struct_and_access(v2))));
  int v_7 = asint((asuint(v_6) + asuint(accept_ptr_to_struct_and_access(v2))));
  int v_8 = asint((asuint(v_7) + asuint(accept_ptr_vec_access_elements(v4))));
  int v_9 = asint((asuint(v_8) + asuint(accept_ptr_to_struct_access_pass_ptr(v2))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(v_9) + asuint(call_builtin_with_mod_scope_ptr())))) + asuint(t1)))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

