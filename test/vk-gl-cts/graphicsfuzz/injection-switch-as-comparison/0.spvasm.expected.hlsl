cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float makeFrame_() {
  float x_60 = 0.0f;
  float x_63_phi = 0.0f;
  while (true) {
    bool x_41 = false;
    float x_44 = 0.0f;
    float x_45 = 0.0f;
    bool x_42 = false;
    bool x_41_phi = false;
    int x_8_phi = 0;
    float x_44_phi = 0.0f;
    float x_60_phi = 0.0f;
    bool x_61_phi = false;
    x_41_phi = false;
    x_8_phi = 0;
    x_44_phi = 0.0f;
    while (true) {
      float x_50 = 0.0f;
      int x_9 = 0;
      bool x_52 = false;
      int x_7 = 0;
      float x_50_phi = 0.0f;
      int x_9_phi = 0;
      bool x_52_phi = false;
      float x_45_phi = 0.0f;
      bool x_42_phi = false;
      x_41 = x_41_phi;
      const int x_8 = x_8_phi;
      x_44 = x_44_phi;
      x_60_phi = x_44;
      x_61_phi = x_41;
      if ((x_8 < 1)) {
      } else {
        break;
      }
      x_50_phi = x_44;
      x_9_phi = x_8;
      x_52_phi = x_41;
      while (true) {
        x_50 = x_50_phi;
        x_9 = x_9_phi;
        x_52 = x_52_phi;
        const float x_54 = asfloat(x_6[0].y);
        x_45_phi = x_50;
        x_42_phi = x_52;
        if ((1 < int(x_54))) {
        } else {
          break;
        }
        x_45_phi = 1.0f;
        x_42_phi = true;
        break;
        {
          x_50_phi = 0.0f;
          x_9_phi = 0;
          x_52_phi = false;
        }
      }
      x_45 = x_45_phi;
      x_42 = x_42_phi;
      x_60_phi = x_45;
      x_61_phi = x_42;
      if (x_42) {
        break;
      }
      {
        x_7 = asint((x_9 + asint(1)));
        x_41_phi = x_42;
        x_8_phi = x_7;
        x_44_phi = x_45;
      }
    }
    x_60 = x_60_phi;
    const bool x_61 = x_61_phi;
    x_63_phi = x_60;
    if (x_61) {
      break;
    }
    x_63_phi = 1.0f;
    break;
  }
  return x_63_phi;
}

void main_1() {
  const float x_34 = makeFrame_();
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
