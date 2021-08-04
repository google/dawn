static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};

void main_1() {
  bool x_30 = false;
  bool x_31_phi = false;
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_23 = asfloat(x_5[0].x);
  const bool x_24 = (x_23 < 0.0f);
  x_31_phi = x_24;
  if (!(x_24)) {
    const float x_29 = asfloat(x_5[0].x);
    x_30 = (x_29 < 1.0f);
    x_31_phi = x_30;
  }
  if (x_31_phi) {
    return;
  }
  const float x_35 = asfloat(x_5[0].x);
  if ((x_35 < 0.0f)) {
    while (true) {
      const float x_45 = asfloat(x_5[0].x);
      if ((x_45 < 0.0f)) {
      } else {
        break;
      }
      x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
      break;
    }
  } else {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
