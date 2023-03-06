float4 tint_acosh(float4 x) {
  return log((x + sqrt(((x * x) - 1.0f))));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void acosh_d51ccb() {
  float4 arg_0 = (1.54308068752288818359f).xxxx;
  float4 res = tint_acosh(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acosh_d51ccb();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acosh_d51ccb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acosh_d51ccb();
  return;
}
