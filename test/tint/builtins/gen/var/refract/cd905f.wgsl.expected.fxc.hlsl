RWByteAddressBuffer prevent_dce : register(u0, space2);

void refract_cd905f() {
  float2 arg_0 = (1.0f).xx;
  float2 arg_1 = (1.0f).xx;
  float arg_2 = 1.0f;
  float2 res = refract(arg_0, arg_1, arg_2);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  refract_cd905f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  refract_cd905f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  refract_cd905f();
  return;
}
