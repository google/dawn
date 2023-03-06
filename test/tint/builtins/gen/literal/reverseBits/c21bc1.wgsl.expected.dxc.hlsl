RWByteAddressBuffer prevent_dce : register(u0, space2);

void reverseBits_c21bc1() {
  int3 res = (-2147483648).xxx;
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  reverseBits_c21bc1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  reverseBits_c21bc1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_c21bc1();
  return;
}
