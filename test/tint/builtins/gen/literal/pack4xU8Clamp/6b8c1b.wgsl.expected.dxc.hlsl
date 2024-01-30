RWByteAddressBuffer prevent_dce : register(u0, space2);

void pack4xU8Clamp_6b8c1b() {
  uint res = 16843009u;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pack4xU8Clamp_6b8c1b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pack4xU8Clamp_6b8c1b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack4xU8Clamp_6b8c1b();
  return;
}
