RWByteAddressBuffer prevent_dce : register(u0, space2);

void min_46c5d3() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = min(arg_0, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  min_46c5d3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  min_46c5d3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_46c5d3();
  return;
}
