RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_9ca42c() {
  vector<float16_t, 2> res = vector<float16_t, 2>(float16_t(0.0h), float16_t(1.875h));
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_9ca42c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_9ca42c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_9ca42c();
  return;
}
