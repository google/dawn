cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_() {
  float b = 0.0f;
  float x_34 = 0.0f;
  float x_34_phi = 0.0f;
  float x_48_phi = 0.0f;
  b = 2.0f;
  x_34_phi = 2.0f;
  while (true) {
    x_34 = x_34_phi;
    const float x_39 = asfloat(x_7[0].x);
    if ((x_39 == 0.0f)) {
      x_48_phi = x_34;
      break;
    }
    const float x_44 = asfloat(x_7[0].x);
    if ((x_44 == 0.0f)) {
      return 1.0f;
    }
    b = 1.0f;
    {
      x_34_phi = 1.0f;
      x_48_phi = 1.0f;
      if (false) {
      } else {
        break;
      }
    }
  }
  return x_48_phi;
}

void main_1() {
  const float x_27 = func_();
  if ((x_27 == 1.0f)) {
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
