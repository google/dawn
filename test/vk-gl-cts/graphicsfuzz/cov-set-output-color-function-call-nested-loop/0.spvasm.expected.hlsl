cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[6];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_36 = 0;
  bool x_74 = false;
  float4 x_33_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int x_36_phi = 0;
  int x_38_phi = 0;
  bool x_75_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  const int x_31 = asint(x_5[1].x);
  x_33_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_36_phi = x_29;
  x_38_phi = x_31;
  while (true) {
    float4 x_53 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int x_39 = 0;
    float4 x_34_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int x_62_phi = 0;
    const float4 x_33 = x_33_phi;
    x_36 = x_36_phi;
    const int x_38 = x_38_phi;
    const int x_41 = asint(x_5[4].x);
    if ((x_38 < x_41)) {
    } else {
      break;
    }
    float4 x_53_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int x_56_phi = 0;
    switch(0u) {
      default: {
        const int x_48 = asint(x_5[3].x);
        if ((x_38 > x_48)) {
          x_34_phi = x_33;
          x_62_phi = 2;
          break;
        }
        x_53_phi = x_33;
        x_56_phi = x_29;
        while (true) {
          float4 x_54 = float4(0.0f, 0.0f, 0.0f, 0.0f);
          int x_57 = 0;
          x_53 = x_53_phi;
          const int x_56 = x_56_phi;
          if ((x_56 < x_41)) {
          } else {
            break;
          }
          {
            const float x_61 = float((x_38 + x_56));
            x_54 = float4(x_61, x_61, x_61, x_61);
            x_57 = (x_56 + 1);
            x_53_phi = x_54;
            x_56_phi = x_57;
          }
        }
        x_GLF_color = x_53;
        x_34_phi = x_53;
        x_62_phi = x_31;
        break;
      }
    }
    const float4 x_34 = x_34_phi;
    const int x_62 = x_62_phi;
    {
      x_39 = (x_38 + 1);
      x_33_phi = x_34;
      x_36_phi = asint((x_36 + asint(x_62)));
      x_38_phi = x_39;
    }
  }
  const float4 x_63 = x_GLF_color;
  const int x_65 = asint(x_5[2].x);
  const float x_66 = float(x_65);
  const bool x_69 = all((x_63 == float4(x_66, x_66, x_66, x_66)));
  x_75_phi = x_69;
  if (x_69) {
    const int x_73 = asint(x_5[5].x);
    x_74 = (x_36 == asint(x_73));
    x_75_phi = x_74;
  }
  if (x_75_phi) {
    const float x_79 = float(x_31);
    const float x_80 = float(x_29);
    x_GLF_color = float4(x_79, x_80, x_80, x_79);
  } else {
    const float x_82 = float(x_29);
    x_GLF_color = float4(x_82, x_82, x_82, x_82);
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
