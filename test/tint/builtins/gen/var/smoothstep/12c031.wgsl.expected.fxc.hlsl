SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void smoothstep_12c031() {
  vector<float16_t, 2> arg_0 = (float16_t(2.0h)).xx;
  vector<float16_t, 2> arg_1 = (float16_t(4.0h)).xx;
  vector<float16_t, 2> arg_2 = (float16_t(3.0h)).xx;
  vector<float16_t, 2> res = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  smoothstep_12c031();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  smoothstep_12c031();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_12c031();
  return;
}
