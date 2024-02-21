RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_66e93d() {
  vector<float16_t, 2> res = vector<float16_t, 2>(float16_t(0.00000005960464477539h), float16_t(0.0h));
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_66e93d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_66e93d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_66e93d();
  return;
}
