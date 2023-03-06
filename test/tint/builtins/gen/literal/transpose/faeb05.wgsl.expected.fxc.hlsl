SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void prevent_dce_store(uint offset, matrix<float16_t, 4, 2> value) {
  prevent_dce.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  prevent_dce.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  prevent_dce.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  prevent_dce.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
}

void transpose_faeb05() {
  matrix<float16_t, 4, 2> res = matrix<float16_t, 4, 2>((float16_t(1.0h)).xx, (float16_t(1.0h)).xx, (float16_t(1.0h)).xx, (float16_t(1.0h)).xx);
  prevent_dce_store(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_faeb05();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_faeb05();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_faeb05();
  return;
}
