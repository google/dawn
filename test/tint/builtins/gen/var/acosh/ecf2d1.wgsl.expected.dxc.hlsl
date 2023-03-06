float tint_acosh(float x) {
  return log((x + sqrt(((x * x) - 1.0f))));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void acosh_ecf2d1() {
  float arg_0 = 1.54308068752288818359f;
  float res = tint_acosh(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acosh_ecf2d1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acosh_ecf2d1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acosh_ecf2d1();
  return;
}
