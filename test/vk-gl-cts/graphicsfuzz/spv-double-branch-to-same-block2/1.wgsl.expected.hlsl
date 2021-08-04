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
  int x_41_phi = 0;
  int x_53_phi = 0;
  x_41_phi = 0;
  while (true) {
    int x_42 = 0;
    const int x_41 = x_41_phi;
    if ((x_41 < 10)) {
    } else {
      break;
    }
    {
      const float x_49 = asfloat(x_9[0].y);
      data[x_41] = (float((10 - x_41)) * x_49);
      x_42 = (x_41 + 1);
      x_41_phi = x_42;
    }
  }
  x_53_phi = 0;
  while (true) {
    int x_54 = 0;
    int x_60_phi = 0;
    const int x_53 = x_53_phi;
    if ((x_53 < 9)) {
    } else {
      break;
    }
    x_60_phi = 0;
    while (true) {
      bool x_83 = false;
      bool x_84 = false;
      int x_61 = 0;
      bool x_85_phi = false;
      const int x_60 = x_60_phi;
      if ((x_60 < 10)) {
      } else {
        break;
      }
      if ((x_60 < (x_53 + 1))) {
        {
          x_61 = (x_60 + 1);
          x_60_phi = x_61;
        }
        continue;
      }
      const int x_70_save = x_53;
      const float x_71 = data[x_70_save];
      const int x_72_save = x_60;
      const float x_73 = data[x_72_save];
      const float x_75 = gl_FragCoord.y;
      const float x_77 = asfloat(x_6[0].y);
      if ((x_75 < (x_77 * 0.5f))) {
        x_83 = (x_71 > x_73);
        x_85_phi = x_83;
      } else {
        x_84 = (x_71 < x_73);
        x_85_phi = x_84;
      }
      if (x_85_phi) {
        const float x_88 = data[x_70_save];
        const float x_89 = data[x_72_save];
        data[x_70_save] = x_89;
        data[x_72_save] = x_88;
      }
      {
        x_61 = (x_60 + 1);
        x_60_phi = x_61;
      }
    }
    {
      x_54 = (x_53 + 1);
      x_53_phi = x_54;
    }
  }
  const float x_91 = gl_FragCoord.x;
  const float x_93 = asfloat(x_6[0].x);
  if ((x_91 < (x_93 * 0.5f))) {
    const float x_100 = data[0];
    const float x_103 = data[5];
    const float x_106 = data[9];
    x_GLF_color = float4((x_100 * 0.100000001f), (x_103 * 0.100000001f), (x_106 * 0.100000001f), 1.0f);
  } else {
    const float x_110 = data[5];
    const float x_113 = data[9];
    const float x_116 = data[0];
    x_GLF_color = float4((x_110 * 0.100000001f), (x_113 * 0.100000001f), (x_116 * 0.100000001f), 1.0f);
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
