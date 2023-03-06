int2 tint_clamp(int2 e, int2 low, int2 high) {
  return min(max(e, low), high);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void clamp_6c1749() {
  int2 arg_0 = (1).xx;
  int2 arg_1 = (1).xx;
  int2 arg_2 = (1).xx;
  int2 res = tint_clamp(arg_0, arg_1, arg_2);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  clamp_6c1749();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  clamp_6c1749();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_6c1749();
  return;
}
