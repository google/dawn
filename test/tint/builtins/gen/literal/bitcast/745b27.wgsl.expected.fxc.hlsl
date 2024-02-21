RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_745b27() {
  float4 res = (1.0f).xxxx;
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_745b27();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_745b27();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_745b27();
  return;
}
