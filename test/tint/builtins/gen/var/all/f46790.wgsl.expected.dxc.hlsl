RWByteAddressBuffer prevent_dce : register(u0, space2);

void all_f46790() {
  bool2 arg_0 = (true).xx;
  bool res = all(arg_0);
  prevent_dce.Store(0u, asuint((all((res == false)) ? 1 : 0)));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  all_f46790();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  all_f46790();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  all_f46790();
  return;
}
