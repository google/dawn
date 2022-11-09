void pack2x16float_0e97b3() {
  uint res = 1006648320u;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pack2x16float_0e97b3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pack2x16float_0e97b3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack2x16float_0e97b3();
  return;
}
