RWByteAddressBuffer prevent_dce : register(u0, space2);

void inverseSqrt_84407e() {
  float arg_0 = 1.0f;
  float res = rsqrt(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  inverseSqrt_84407e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  inverseSqrt_84407e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  inverseSqrt_84407e();
  return;
}
