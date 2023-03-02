void unpack2x16snorm_b4aea6() {
  float2 res = float2(0.00003051850944757462f, 0.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack2x16snorm_b4aea6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack2x16snorm_b4aea6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16snorm_b4aea6();
  return;
}
