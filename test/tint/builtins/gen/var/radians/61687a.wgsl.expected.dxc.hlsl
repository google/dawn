float2 tint_radians(float2 param_0) {
  return param_0 * 0.01745329251994329547;
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void radians_61687a() {
  float2 arg_0 = (1.0f).xx;
  float2 res = tint_radians(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_61687a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_61687a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_61687a();
  return;
}
