static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float data[10] = (float[10])0;
  int x_40_phi = 0;
  int x_52_phi = 0;
  x_40_phi = 0;
  while (true) {
    int x_41 = 0;
    const int x_40 = x_40_phi;
    if ((x_40 < 10)) {
    } else {
      break;
    }
    {
      const float x_48 = asfloat(x_9[0].y);
      data[x_40] = (float((10 - x_40)) * x_48);
      x_41 = (x_40 + 1);
      x_40_phi = x_41;
    }
  }
  x_52_phi = 0;
  while (true) {
    int x_53 = 0;
    int x_59_phi = 0;
    const int x_52 = x_52_phi;
    if ((x_52 < 9)) {
    } else {
      break;
    }
    x_59_phi = 0;
    while (true) {
      bool x_82 = false;
      bool x_83 = false;
      int x_60 = 0;
      bool x_84_phi = false;
      const int x_59 = x_59_phi;
      if ((x_59 < 10)) {
      } else {
        break;
      }
      if ((x_59 < (x_52 + 1))) {
        {
          x_60 = (x_59 + 1);
          x_59_phi = x_60;
        }
        continue;
      }
      const int x_69_save = x_52;
      const float x_70 = data[x_69_save];
      const int x_71_save = x_59;
      const float x_72 = data[x_71_save];
      const float x_74 = gl_FragCoord.y;
      const float x_76 = asfloat(x_6[0].y);
      if ((x_74 < (x_76 * 0.5f))) {
        x_82 = (x_70 > x_72);
        x_84_phi = x_82;
      } else {
        x_83 = (x_70 < x_72);
        x_84_phi = x_83;
      }
      if (x_84_phi) {
        const float x_87 = data[x_69_save];
        const float x_88 = data[x_71_save];
        data[x_69_save] = x_88;
        data[x_71_save] = x_87;
      }
      {
        x_60 = (x_59 + 1);
        x_59_phi = x_60;
      }
    }
    {
      x_53 = (x_52 + 1);
      x_52_phi = x_53;
    }
  }
  const float x_90 = gl_FragCoord.x;
  const float x_92 = asfloat(x_6[0].x);
  if ((x_90 < (x_92 * 0.5f))) {
    const float x_99 = data[0];
    const float x_102 = data[5];
    const float x_105 = data[9];
    x_GLF_color = float4((x_99 * 0.100000001f), (x_102 * 0.100000001f), (x_105 * 0.100000001f), 1.0f);
  } else {
    const float x_109 = data[5];
    const float x_112 = data[9];
    const float x_115 = data[0];
    x_GLF_color = float4((x_109 * 0.100000001f), (x_112 * 0.100000001f), (x_115 * 0.100000001f), 1.0f);
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
