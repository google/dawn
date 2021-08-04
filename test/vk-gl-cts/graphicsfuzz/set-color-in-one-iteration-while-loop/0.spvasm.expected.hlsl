static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};

void main_1() {
  while (true) {
    float4 x_42 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bool x_39 = false;
    bool x_38_phi = false;
    float4 x_41_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float x_32 = gl_FragCoord.x;
    const int x_34 = int(clamp(x_32, 0.0f, 1.0f));
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    x_38_phi = false;
    x_41_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    while (true) {
      float4 x_42_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
      int x_47_phi = 0;
      bool x_39_phi = false;
      const bool x_38 = x_38_phi;
      x_42_phi = x_41_phi;
      x_47_phi = 0;
      while (true) {
        float4 x_45 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        int x_48 = 0;
        x_42 = x_42_phi;
        const int x_47 = x_47_phi;
        const float x_50 = asfloat(x_6[0].y);
        x_39_phi = x_38;
        if ((x_47 < (x_34 + int(x_50)))) {
        } else {
          break;
        }
        float4 x_66 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 x_70 = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 x_45_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
        if ((x_34 < 0)) {
          x_39_phi = true;
          break;
        } else {
          if ((x_34 == 1)) {
            const float x_64 = float(x_34);
            const float2 x_65 = float2(x_64, x_64);
            x_66 = float4(x_65.x, x_42.y, x_42.z, x_65.y);
            x_45_phi = x_66;
          } else {
            const float x_68 = float((x_34 + 1));
            const float2 x_69 = float2(x_68, x_68);
            x_70 = float4(x_69.x, x_42.y, x_42.z, x_69.y);
            x_45_phi = x_70;
          }
          x_45 = x_45_phi;
        }
        {
          x_48 = (x_47 + 1);
          x_42_phi = x_45;
          x_47_phi = x_48;
        }
      }
      x_39 = x_39_phi;
      if (x_39) {
        break;
      }
      {
        x_38_phi = x_39;
        x_41_phi = x_42;
        if ((x_34 < 0)) {
        } else {
          break;
        }
      }
    }
    if (x_39) {
      break;
    }
    x_GLF_color = x_42;
    break;
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
