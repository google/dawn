void unpack4x8snorm_523fb3() {
  float4 res = float4(0.007874016f, 0.0f, 0.0f, 0.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack4x8snorm_523fb3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack4x8snorm_523fb3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4x8snorm_523fb3();
  return;
}
