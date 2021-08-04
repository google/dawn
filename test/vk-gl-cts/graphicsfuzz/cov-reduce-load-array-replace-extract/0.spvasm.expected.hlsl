static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};

void main_1() {
  int x_9[1] = (int[1])0;
  int x_10_phi = 0;
  const int x_6 = x_9[0u];
  while (true) {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const int x_7 = asint(x_5[0].x);
    const int x_8 = x_9[x_7];
    if ((x_8 == x_6)) {
      x_10_phi = 1;
      break;
    }
    x_10_phi = 2;
    break;
  }
  const int x_10 = x_10_phi;
  if (((x_10 == 1) | (x_10 == 2))) {
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
