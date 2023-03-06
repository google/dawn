RWByteAddressBuffer prevent_dce : register(u0, space2);

void tanh_c15fdb() {
  float res = 0.76159417629241943359f;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tanh_c15fdb();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tanh_c15fdb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_c15fdb();
  return;
}
