SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void prevent_dce_store(uint offset, matrix<float16_t, 2, 2> value) {
  prevent_dce.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  prevent_dce.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
}

void transpose_7be8b2() {
  matrix<float16_t, 2, 2> arg_0 = matrix<float16_t, 2, 2>((float16_t(1.0h)).xx, (float16_t(1.0h)).xx);
  matrix<float16_t, 2, 2> res = transpose(arg_0);
  prevent_dce_store(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_7be8b2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_7be8b2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_7be8b2();
  return;
}
