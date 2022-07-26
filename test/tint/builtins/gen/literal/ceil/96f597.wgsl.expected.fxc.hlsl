void ceil_96f597() {
  float2 res = ceil((1.0f).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ceil_96f597();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ceil_96f597();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_96f597();
  return;
}
