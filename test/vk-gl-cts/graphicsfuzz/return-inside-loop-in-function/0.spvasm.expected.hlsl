static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_36 = false;
  float3 x_37 = float3(0.0f, 0.0f, 0.0f);
  int x_6 = 0;
  float3 x_38 = float3(0.0f, 0.0f, 0.0f);
  float3 x_51 = float3(0.0f, 0.0f, 0.0f);
  float3 x_54 = float3(0.0f, 0.0f, 0.0f);
  bool x_40_phi = false;
  float3 x_55_phi = float3(0.0f, 0.0f, 0.0f);
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  x_36 = false;
  x_40_phi = false;
  while (true) {
    bool x_45 = false;
    bool x_45_phi = false;
    int x_7_phi = 0;
    float3 x_51_phi = float3(0.0f, 0.0f, 0.0f);
    bool x_52_phi = false;
    const bool x_40 = x_40_phi;
    x_6 = 0;
    x_45_phi = x_40;
    x_7_phi = 0;
    while (true) {
      x_45 = x_45_phi;
      const int x_7 = x_7_phi;
      x_51_phi = float3(0.0f, 0.0f, 0.0f);
      x_52_phi = x_45;
      if ((x_7 < 0)) {
      } else {
        break;
      }
      x_36 = true;
      x_37 = float3(1.0f, 1.0f, 1.0f);
      x_51_phi = float3(1.0f, 1.0f, 1.0f);
      x_52_phi = true;
      break;
      {
        x_45_phi = false;
        x_7_phi = 0;
      }
    }
    x_51 = x_51_phi;
    const bool x_52 = x_52_phi;
    x_55_phi = x_51;
    if (x_52) {
      break;
    }
    x_54 = float3(0.0f, 0.0f, 0.0f);
    x_36 = true;
    x_55_phi = x_54;
    break;
    {
      x_40_phi = false;
    }
  }
  x_38 = x_55_phi;
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

float3 GLF_live4drawShape_() {
  bool x_57 = false;
  float3 x_58 = float3(0.0f, 0.0f, 0.0f);
  int i = 0;
  float3 x_71 = float3(0.0f, 0.0f, 0.0f);
  float3 x_74 = float3(0.0f, 0.0f, 0.0f);
  bool x_60_phi = false;
  float3 x_75_phi = float3(0.0f, 0.0f, 0.0f);
  x_60_phi = false;
  while (true) {
    bool x_65 = false;
    bool x_65_phi = false;
    int x_8_phi = 0;
    float3 x_71_phi = float3(0.0f, 0.0f, 0.0f);
    bool x_72_phi = false;
    const bool x_60 = x_60_phi;
    i = 0;
    x_65_phi = x_60;
    x_8_phi = 0;
    while (true) {
      x_65 = x_65_phi;
      const int x_8 = x_8_phi;
      x_71_phi = float3(0.0f, 0.0f, 0.0f);
      x_72_phi = x_65;
      if ((x_8 < 0)) {
      } else {
        break;
      }
      x_57 = true;
      x_58 = float3(1.0f, 1.0f, 1.0f);
      x_71_phi = float3(1.0f, 1.0f, 1.0f);
      x_72_phi = true;
      break;
      {
        x_65_phi = false;
        x_8_phi = 0;
      }
    }
    x_71 = x_71_phi;
    const bool x_72 = x_72_phi;
    x_75_phi = x_71;
    if (x_72) {
      break;
    }
    x_74 = float3(0.0f, 0.0f, 0.0f);
    x_57 = true;
    x_75_phi = x_74;
    break;
    {
      x_60_phi = false;
    }
  }
  return x_75_phi;
}
