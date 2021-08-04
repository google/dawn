static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 m = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  m = (transpose(float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))) * (1.0f / 2.0f));
  const float2x2 x_33 = m;
  bool tint_tmp = all((x_33[0u] == float2x2(float2(0.5f, 1.5f), float2(1.0f, 2.0f))[0u]));
  if (tint_tmp) {
    tint_tmp = all((x_33[1u] == float2x2(float2(0.5f, 1.5f), float2(1.0f, 2.0f))[1u]));
  }
  if ((tint_tmp)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
