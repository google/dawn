void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float3 c = float3(0.0f, 0.0f, 0.0f);
  float x_54 = 0.0f;
  float x_58 = 0.0f;
  float x_59 = 0.0f;
  float x_91 = 0.0f;
  float x_92 = 0.0f;
  float x_135 = 0.0f;
  float x_136 = 0.0f;
  float x_58_phi = 0.0f;
  int x_61_phi = 0;
  float x_91_phi = 0.0f;
  float x_92_phi = 0.0f;
  bool x_93_phi = false;
  float x_95_phi = 0.0f;
  float x_139_phi = 0.0f;
  int x_146_phi = 0;
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_50 = asfloat(x_9[0].x);
  const float x_52 = round((x_50 * 0.125f));
  x_54 = gl_FragCoord.x;
  switch(0u) {
    default: {
      x_58_phi = -0.5f;
      x_61_phi = 1;
      while (true) {
        float x_71 = 0.0f;
        float x_79 = 0.0f;
        int x_62 = 0;
        float x_59_phi = 0.0f;
        x_58 = x_58_phi;
        const int x_61 = x_61_phi;
        x_91_phi = 0.0f;
        x_92_phi = x_58;
        x_93_phi = false;
        if ((x_61 < 800)) {
        } else {
          break;
        }
        float x_78 = 0.0f;
        float x_79_phi = 0.0f;
        if (((x_61 % 32) == 0)) {
          x_71 = (x_58 + 0.400000006f);
          x_59_phi = x_71;
        } else {
          x_79_phi = x_58;
          if (((float(x_61) - (round(x_52) * floor((float(x_61) / round(x_52))))) <= 0.01f)) {
            x_78 = (x_58 + 100.0f);
            x_79_phi = x_78;
          }
          x_79 = x_79_phi;
          const float x_81 = asfloat(x_6[0].x);
          const float x_83 = asfloat(x_6[0].y);
          if ((x_81 > x_83)) {
            discard;
          }
          x_59_phi = x_79;
        }
        x_59 = x_59_phi;
        if ((float(x_61) >= x_54)) {
          x_91_phi = x_59;
          x_92_phi = x_59;
          x_93_phi = true;
          break;
        }
        {
          x_62 = (x_61 + 1);
          x_58_phi = x_59;
          x_61_phi = x_62;
        }
      }
      x_91 = x_91_phi;
      x_92 = x_92_phi;
      const bool x_93 = x_93_phi;
      x_95_phi = x_91;
      if (x_93) {
        break;
      }
      x_95_phi = x_92;
      break;
    }
  }
  float x_98 = 0.0f;
  float x_102 = 0.0f;
  float x_103 = 0.0f;
  float x_102_phi = 0.0f;
  int x_105_phi = 0;
  float x_135_phi = 0.0f;
  float x_136_phi = 0.0f;
  bool x_137_phi = false;
  const float x_95 = x_95_phi;
  c.x = x_95;
  x_98 = gl_FragCoord.y;
  switch(0u) {
    default: {
      x_102_phi = -0.5f;
      x_105_phi = 1;
      while (true) {
        float x_115 = 0.0f;
        float x_123 = 0.0f;
        int x_106 = 0;
        float x_103_phi = 0.0f;
        x_102 = x_102_phi;
        const int x_105 = x_105_phi;
        x_135_phi = 0.0f;
        x_136_phi = x_102;
        x_137_phi = false;
        if ((x_105 < 800)) {
        } else {
          break;
        }
        float x_122 = 0.0f;
        float x_123_phi = 0.0f;
        if (((x_105 % 32) == 0)) {
          x_115 = (x_102 + 0.400000006f);
          x_103_phi = x_115;
        } else {
          x_123_phi = x_102;
          if (((float(x_105) - (round(x_52) * floor((float(x_105) / round(x_52))))) <= 0.01f)) {
            x_122 = (x_102 + 100.0f);
            x_123_phi = x_122;
          }
          x_123 = x_123_phi;
          const float x_125 = asfloat(x_6[0].x);
          const float x_127 = asfloat(x_6[0].y);
          if ((x_125 > x_127)) {
            discard;
          }
          x_103_phi = x_123;
        }
        x_103 = x_103_phi;
        if ((float(x_105) >= x_98)) {
          x_135_phi = x_103;
          x_136_phi = x_103;
          x_137_phi = true;
          break;
        }
        {
          x_106 = (x_105 + 1);
          x_102_phi = x_103;
          x_105_phi = x_106;
        }
      }
      x_135 = x_135_phi;
      x_136 = x_136_phi;
      const bool x_137 = x_137_phi;
      x_139_phi = x_135;
      if (x_137) {
        break;
      }
      x_139_phi = x_136;
      break;
    }
  }
  const float x_139 = x_139_phi;
  c.y = x_139;
  const float x_141 = c.x;
  const float x_142 = c.y;
  c.z = (x_141 + x_142);
  x_146_phi = 0;
  while (true) {
    int x_147 = 0;
    const int x_146 = x_146_phi;
    if ((x_146 < 3)) {
    } else {
      break;
    }
    const int x_152_save = x_146;
    const float x_153 = c[x_152_save];
    if ((x_153 >= 1.0f)) {
      const float x_157 = c[x_152_save];
      const float x_158 = c[x_152_save];
      set_float3(c, x_152_save, (x_157 * x_158));
      const float x_161 = asfloat(x_6[0].x);
      const float x_163 = asfloat(x_6[0].y);
      if ((x_161 > x_163)) {
        discard;
      }
    }
    {
      x_147 = (x_146 + 1);
      x_146_phi = x_147;
    }
  }
  const float3 x_169 = normalize(abs(c));
  x_GLF_color = float4(x_169.x, x_169.y, x_169.z, 1.0f);
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
