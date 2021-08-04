cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void x_51() {
  discard;
}

void main_1() {
  while (true) {
    bool x_31 = false;
    bool x_30_phi = false;
    x_30_phi = false;
    while (true) {
      bool x_31_phi = false;
      const bool x_30 = x_30_phi;
      while (true) {
        float4 x_52 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 x_54 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 x_55_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
        const float x_36 = asfloat(x_5[0].y);
        x_31_phi = x_30;
        if ((x_36 > 0.0f)) {
        } else {
          break;
        }
        while (true) {
          const float x_46 = asfloat(x_5[0].x);
          if ((x_46 > 0.0f)) {
            x_51();
            x_52 = float4(0.0f, 0.0f, 0.0f, 0.0f);
            x_55_phi = x_52;
            break;
          }
          x_54 = (float4(1.0f, 0.0f, 0.0f, 1.0f) + float4(x_46, x_46, x_46, x_46));
          x_55_phi = x_54;
          break;
        }
        x_GLF_color = x_55_phi;
        x_31_phi = true;
        break;
      }
      x_31 = x_31_phi;
      if (x_31) {
        break;
      } else {
        {
          x_30_phi = x_31;
        }
        continue;
      }
      {
        x_30_phi = x_31;
      }
    }
    if (x_31) {
      break;
    }
    break;
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
