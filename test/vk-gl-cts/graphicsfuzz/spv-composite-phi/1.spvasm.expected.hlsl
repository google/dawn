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
  float x_53 = 0.0f;
  float x_57 = 0.0f;
  float x_58 = 0.0f;
  float x_83 = 0.0f;
  float x_84 = 0.0f;
  float x_124 = 0.0f;
  float x_125 = 0.0f;
  float x_57_phi = 0.0f;
  int x_60_phi = 0;
  float x_83_phi = 0.0f;
  float x_84_phi = 0.0f;
  bool x_85_phi = false;
  float x_87_phi = 0.0f;
  float x_128_phi = 0.0f;
  int x_135_phi = 0;
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_47 = asfloat(x_7[0].x);
  const float2 x_48 = float2(1.0f, x_47);
  const float x_50 = round((x_47 * 0.125f));
  const float2 x_51 = float2(0.0f, -0.5f);
  x_53 = gl_FragCoord.x;
  switch(0u) {
    default: {
      x_57_phi = -0.5f;
      x_60_phi = 1;
      while (true) {
        float x_70 = 0.0f;
        float x_78 = 0.0f;
        int x_61 = 0;
        float x_58_phi = 0.0f;
        x_57 = x_57_phi;
        const int x_60 = x_60_phi;
        x_83_phi = 0.0f;
        x_84_phi = x_57;
        x_85_phi = false;
        if ((x_60 < 800)) {
        } else {
          break;
        }
        float x_77 = 0.0f;
        float x_78_phi = 0.0f;
        if (((x_60 % 32) == 0)) {
          x_70 = (x_57 + 0.400000006f);
          x_58_phi = x_70;
        } else {
          x_78_phi = x_57;
          if (((float(x_60) - (round(x_50) * floor((float(x_60) / round(x_50))))) <= 0.01f)) {
            x_77 = (x_57 + 100.0f);
            x_78_phi = x_77;
          }
          x_78 = x_78_phi;
          x_58_phi = x_78;
        }
        x_58 = x_58_phi;
        if ((float(x_60) >= x_53)) {
          x_83_phi = x_58;
          x_84_phi = x_58;
          x_85_phi = true;
          break;
        }
        {
          x_61 = (x_60 + 1);
          x_57_phi = x_58;
          x_60_phi = x_61;
        }
      }
      x_83 = x_83_phi;
      x_84 = x_84_phi;
      const bool x_85 = x_85_phi;
      x_87_phi = x_83;
      if (x_85) {
        break;
      }
      x_87_phi = x_84;
      break;
    }
  }
  float x_92 = 0.0f;
  float x_98 = 0.0f;
  float x_99 = 0.0f;
  float x_98_phi = 0.0f;
  int x_101_phi = 0;
  float x_124_phi = 0.0f;
  float x_125_phi = 0.0f;
  bool x_126_phi = false;
  const float x_87 = x_87_phi;
  const float4 x_89 = float4(x_84, 0.400000006f, x_83, 0.400000006f);
  c.x = x_87;
  x_92 = gl_FragCoord.y;
  switch(0u) {
    default: {
      const float4 x_95 = float4(x_51, 0.0f, x_57);
      const float x_96 = float3(x_48, -0.5f).z;
      x_98_phi = x_96;
      x_101_phi = 1;
      while (true) {
        float x_111 = 0.0f;
        float x_119 = 0.0f;
        int x_102 = 0;
        float x_99_phi = 0.0f;
        x_98 = x_98_phi;
        const int x_101 = x_101_phi;
        x_124_phi = 0.0f;
        x_125_phi = x_98;
        x_126_phi = false;
        if ((x_101 < 800)) {
        } else {
          break;
        }
        float x_118 = 0.0f;
        float x_119_phi = 0.0f;
        if (((x_101 % 32) == 0)) {
          x_111 = (x_98 + 0.400000006f);
          x_99_phi = x_111;
        } else {
          x_119_phi = x_98;
          if (((float(x_101) - (round(x_50) * floor((float(x_101) / round(x_50))))) <= 0.01f)) {
            x_118 = (x_98 + 100.0f);
            x_119_phi = x_118;
          }
          x_119 = x_119_phi;
          x_99_phi = x_119;
        }
        x_99 = x_99_phi;
        if ((float(x_101) >= x_92)) {
          x_124_phi = x_99;
          x_125_phi = x_99;
          x_126_phi = true;
          break;
        }
        {
          x_102 = (x_101 + 1);
          x_98_phi = x_99;
          x_101_phi = x_102;
        }
      }
      x_124 = x_124_phi;
      x_125 = x_125_phi;
      const bool x_126 = x_126_phi;
      x_128_phi = x_124;
      if (x_126) {
        break;
      }
      x_128_phi = x_125;
      break;
    }
  }
  const float x_128 = x_128_phi;
  c.y = x_128;
  const float x_130 = c.x;
  const float x_131 = c.y;
  c.z = (x_130 + x_131);
  x_135_phi = 0;
  while (true) {
    int x_136 = 0;
    const int x_135 = x_135_phi;
    if ((x_135 < 3)) {
    } else {
      break;
    }
    const int x_141_save = x_135;
    const float x_142 = c[x_141_save];
    if ((x_142 >= 1.0f)) {
      const float x_146 = c[x_141_save];
      const float x_147 = c[x_141_save];
      set_float3(c, x_141_save, (x_146 * x_147));
    }
    {
      x_136 = (x_135 + 1);
      x_135_phi = x_136;
    }
  }
  const float3 x_151 = normalize(abs(c));
  x_GLF_color = float4(x_151.x, x_151.y, x_151.z, 1.0f);
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
