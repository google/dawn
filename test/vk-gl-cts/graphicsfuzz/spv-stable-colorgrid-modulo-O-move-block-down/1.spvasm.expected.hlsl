void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float3 c = float3(0.0f, 0.0f, 0.0f);
  float x_51 = 0.0f;
  float x_55 = 0.0f;
  float x_56 = 0.0f;
  float x_81 = 0.0f;
  float x_82 = 0.0f;
  float x_118 = 0.0f;
  float x_119 = 0.0f;
  float x_55_phi = 0.0f;
  int x_58_phi = 0;
  float x_81_phi = 0.0f;
  float x_82_phi = 0.0f;
  bool x_83_phi = false;
  float x_85_phi = 0.0f;
  float x_122_phi = 0.0f;
  int x_129_phi = 0;
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_47 = asfloat(x_7[0].x);
  const float x_49 = round((x_47 * 0.125f));
  x_51 = gl_FragCoord.x;
  switch(0u) {
    default: {
      x_55_phi = -0.5f;
      x_58_phi = 1;
      while (true) {
        float x_68 = 0.0f;
        float x_76 = 0.0f;
        int x_59 = 0;
        float x_56_phi = 0.0f;
        x_55 = x_55_phi;
        const int x_58 = x_58_phi;
        x_81_phi = 0.0f;
        x_82_phi = x_55;
        x_83_phi = false;
        if ((x_58 < 800)) {
        } else {
          break;
        }
        float x_75 = 0.0f;
        float x_76_phi = 0.0f;
        if (((x_58 % 32) == 0)) {
          x_68 = (x_55 + 0.400000006f);
          x_56_phi = x_68;
        } else {
          x_76_phi = x_55;
          if (((float(x_58) - (round(x_49) * floor((float(x_58) / round(x_49))))) <= 0.01f)) {
            x_75 = (x_55 + 100.0f);
            x_76_phi = x_75;
          }
          x_76 = x_76_phi;
          x_56_phi = x_76;
        }
        x_56 = x_56_phi;
        if ((float(x_58) >= x_51)) {
          x_81_phi = x_56;
          x_82_phi = x_56;
          x_83_phi = true;
          break;
        }
        {
          x_59 = (x_58 + 1);
          x_55_phi = x_56;
          x_58_phi = x_59;
        }
      }
      x_81 = x_81_phi;
      x_82 = x_82_phi;
      const bool x_83 = x_83_phi;
      x_85_phi = x_81;
      if (x_83) {
        break;
      }
      x_85_phi = x_82;
      break;
    }
  }
  float x_88 = 0.0f;
  float x_92 = 0.0f;
  float x_93 = 0.0f;
  float x_92_phi = 0.0f;
  int x_95_phi = 0;
  float x_118_phi = 0.0f;
  float x_119_phi = 0.0f;
  bool x_120_phi = false;
  const float x_85 = x_85_phi;
  c.x = x_85;
  x_88 = gl_FragCoord.y;
  switch(0u) {
    default: {
      x_92_phi = -0.5f;
      x_95_phi = 1;
      while (true) {
        float x_113 = 0.0f;
        float x_112 = 0.0f;
        int x_96 = 0;
        float x_93_phi = 0.0f;
        x_92 = x_92_phi;
        const int x_95 = x_95_phi;
        x_118_phi = 0.0f;
        x_119_phi = x_92;
        x_120_phi = false;
        if ((x_95 < 800)) {
        } else {
          break;
        }
        float x_111 = 0.0f;
        float x_112_phi = 0.0f;
        if (((x_95 % 32) == 0)) {
          x_113 = (x_92 + 0.400000006f);
          x_93_phi = x_113;
        } else {
          x_112_phi = x_92;
          if (((float(x_95) - (round(x_49) * floor((float(x_95) / round(x_49))))) <= 0.01f)) {
            x_111 = (x_92 + 100.0f);
            x_112_phi = x_111;
          }
          x_112 = x_112_phi;
          x_93_phi = x_112;
        }
        x_93 = x_93_phi;
        if ((float(x_95) >= x_88)) {
          x_118_phi = x_93;
          x_119_phi = x_93;
          x_120_phi = true;
          break;
        }
        {
          x_96 = (x_95 + 1);
          x_92_phi = x_93;
          x_95_phi = x_96;
        }
      }
      x_118 = x_118_phi;
      x_119 = x_119_phi;
      const bool x_120 = x_120_phi;
      x_122_phi = x_118;
      if (x_120) {
        break;
      }
      x_122_phi = x_119;
      break;
    }
  }
  const float x_122 = x_122_phi;
  c.y = x_122;
  const float x_124 = c.x;
  const float x_125 = c.y;
  c.z = (x_124 + x_125);
  x_129_phi = 0;
  while (true) {
    int x_130 = 0;
    const int x_129 = x_129_phi;
    if ((x_129 < 3)) {
    } else {
      break;
    }
    const int x_135_save = x_129;
    const float x_136 = c[x_135_save];
    if ((x_136 >= 1.0f)) {
      const float x_140 = c[x_135_save];
      const float x_141 = c[x_135_save];
      set_float3(c, x_135_save, (x_140 * x_141));
    }
    {
      x_130 = (x_129 + 1);
      x_129_phi = x_130;
    }
  }
  const float3 x_145 = normalize(abs(c));
  x_GLF_color = float4(x_145.x, x_145.y, x_145.z, 1.0f);
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
