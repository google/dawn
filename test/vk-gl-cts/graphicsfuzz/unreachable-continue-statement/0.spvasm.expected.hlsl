static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};

float3 computeColor_() {
  int x_injected_loop_counter = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  x_injected_loop_counter = 1;
  while (true) {
    const float x_38 = asfloat(x_7[0].x);
    if ((x_38 > 1.0f)) {
      const float x_43 = asfloat(x_7[0].x);
      if ((x_43 > 1.0f)) {
        continue;
      } else {
        continue;
      }
      continue;
    }
    return float3(1.0f, 1.0f, 1.0f);
  }
  return float3(0.0f, 0.0f, 0.0f);
}

void main_1() {
  const float3 x_31 = computeColor_();
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
