RWByteAddressBuffer prevent_dce : register(u0, space2);

void select_b04721() {
  uint3 res = (1u).xxx;
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_b04721();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_b04721();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_b04721();
  return;
}
