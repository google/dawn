cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 returnRed_() {
  bool x_33 = false;
  float4 x_34 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_48 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_36_phi = false;
  float4 x_51_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_36_phi = false;
  while (true) {
    float4 x_48_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bool x_49_phi = false;
    const bool x_36 = x_36_phi;
    while (true) {
      const int x_44 = asint(x_6[0].x);
      if ((x_44 == 1)) {
        x_33 = true;
        x_34 = float4(1.0f, 0.0f, 0.0f, 1.0f);
        x_48_phi = float4(1.0f, 0.0f, 0.0f, 1.0f);
        x_49_phi = true;
        break;
      }
      {
        x_48_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
        x_49_phi = false;
        if (false) {
        } else {
          break;
        }
      }
    }
    x_48 = x_48_phi;
    const bool x_49 = x_49_phi;
    x_51_phi = x_48;
    if (x_49) {
      break;
    }
    x_33 = true;
    x_34 = float4(1.0f, 0.0f, 0.0f, 1.0f);
    x_51_phi = float4(1.0f, 0.0f, 0.0f, 1.0f);
    break;
    {
      x_36_phi = false;
    }
  }
  return x_51_phi;
}

void main_1() {
  while (true) {
    const float4 x_30 = returnRed_();
    x_GLF_color = x_30;
    if (false) {
    } else {
      break;
    }
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
