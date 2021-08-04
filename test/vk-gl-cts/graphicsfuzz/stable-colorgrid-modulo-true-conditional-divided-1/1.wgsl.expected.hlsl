void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_16 : register(b1, space0) {
  uint4 x_16[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float x_104 = 0.0f;
  float x_104_phi = 0.0f;
  int x_107_phi = 0;
  x_104_phi = -0.5f;
  x_107_phi = 1;
  while (true) {
    float x_126 = 0.0f;
    float x_125 = 0.0f;
    int x_108 = 0;
    float x_105_phi = 0.0f;
    x_104 = x_104_phi;
    const int x_107 = x_107_phi;
    if ((x_107 < 800)) {
    } else {
      break;
    }
    float x_124 = 0.0f;
    float x_125_phi = 0.0f;
    if (((x_107 % 32) == 0)) {
      x_126 = (x_104 + 0.400000006f);
      x_105_phi = x_126;
    } else {
      const float x_118 = thirty_two;
      x_125_phi = x_104;
      if (((float(x_107) % round(x_118)) <= 0.01f)) {
        x_124 = (x_104 + 100.0f);
        x_125_phi = x_124;
      }
      x_125 = x_125_phi;
      x_105_phi = x_125;
    }
    float x_105 = 0.0f;
    x_105 = x_105_phi;
    const float x_128 = limit;
    if ((float(x_107) >= x_128)) {
      return x_105;
    }
    {
      x_108 = (x_107 + 1);
      x_104_phi = x_105;
      x_107_phi = x_108;
    }
  }
  return x_104;
}

void main_1() {
  float3 c = float3(0.0f, 0.0f, 0.0f);
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  float3 x_54 = float3(0.0f, 0.0f, 0.0f);
  int x_74_phi = 0;
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_56 = asfloat(x_10[0].x);
  const float x_58 = round((x_56 * 0.125f));
  const float x_60 = gl_FragCoord.x;
  param = x_60;
  param_1 = x_58;
  const float x_61 = compute_value_f1_f1_(param, param_1);
  c.x = x_61;
  const float x_64 = gl_FragCoord.y;
  param_2 = x_64;
  param_3 = x_58;
  const float x_65 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_65;
  const float x_67 = c.x;
  x_54 = c;
  const float x_70 = x_54.y;
  c.z = (x_67 + x_70);
  x_74_phi = 0;
  while (true) {
    int x_75 = 0;
    const int x_74 = x_74_phi;
    if ((x_74 < 3)) {
    } else {
      break;
    }
    const int x_80_save = x_74;
    const float x_81 = c[x_80_save];
    if ((x_81 >= 1.0f)) {
      const float x_86 = asfloat(x_16[0].x);
      const float x_88 = asfloat(x_16[0].y);
      if ((x_86 > x_88)) {
        discard;
      }
      const float x_92 = c[x_80_save];
      const float x_93 = c[x_80_save];
      set_float3(c, x_80_save, (x_92 * x_93));
    }
    {
      x_75 = (x_74 + 1);
      x_74_phi = x_75;
    }
  }
  const float3 x_97 = normalize(abs(c));
  x_GLF_color = float4(x_97.x, x_97.y, x_97.z, 1.0f);
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
