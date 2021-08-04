static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 x_26 = float2(0.0f, 0.0f);
  bool x_39 = false;
  float2 x_26_phi = float2(0.0f, 0.0f);
  int x_5_phi = 0;
  bool x_40_phi = false;
  x_26_phi = float2(0.0f, 0.0f);
  x_5_phi = 2;
  while (true) {
    float2 x_27 = float2(0.0f, 0.0f);
    int x_4 = 0;
    x_26 = x_26_phi;
    const int x_5 = x_5_phi;
    if ((x_5 < 3)) {
    } else {
      break;
    }
    {
      const float2 x_32 = float2(1.0f, float(x_5));
      x_27 = float2(x_32.x, x_32.y);
      x_4 = (x_5 + 1);
      x_26_phi = x_27;
      x_5_phi = x_4;
    }
  }
  const bool x_34 = (x_26.x != 1.0f);
  x_40_phi = x_34;
  if (!(x_34)) {
    x_39 = (x_26.y != 2.0f);
    x_40_phi = x_39;
  }
  if (x_40_phi) {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
