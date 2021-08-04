void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float x_91 = 0.0f;
  float x_91_phi = 0.0f;
  int x_94_phi = 0;
  x_91_phi = -0.5f;
  x_94_phi = 1;
  while (true) {
    float x_104 = 0.0f;
    float x_113 = 0.0f;
    int x_95 = 0;
    float x_92_phi = 0.0f;
    x_91 = x_91_phi;
    const int x_94 = x_94_phi;
    if ((x_94 < 800)) {
    } else {
      break;
    }
    float x_112 = 0.0f;
    float x_113_phi = 0.0f;
    if (((x_94 % 32) == 0)) {
      x_104 = (x_91 + 0.400000006f);
      x_92_phi = x_104;
    } else {
      const float x_106 = thirty_two;
      x_113_phi = x_91;
      if (((float(x_94) % round(x_106)) <= 0.01f)) {
        x_112 = (x_91 + 100.0f);
        x_113_phi = x_112;
      }
      x_113 = x_113_phi;
      x_92_phi = x_113;
    }
    float x_92 = 0.0f;
    x_92 = x_92_phi;
    const float x_115 = limit;
    if ((float(x_94) >= x_115)) {
      return x_92;
    }
    {
      x_95 = (x_94 + 1);
      x_91_phi = x_92;
      x_94_phi = x_95;
    }
  }
  return x_91;
}

void main_1() {
  float3 c = float3(0.0f, 0.0f, 0.0f);
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int x_68_phi = 0;
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_52 = asfloat(x_10[0].x);
  const float x_54 = round((x_52 * 0.125f));
  const float x_56 = gl_FragCoord.x;
  param = x_56;
  param_1 = x_54;
  const float x_57 = compute_value_f1_f1_(param, param_1);
  c.x = x_57;
  const float x_60 = gl_FragCoord.y;
  param_2 = x_60;
  param_3 = x_54;
  const float x_61 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_61;
  const float x_63 = c.x;
  const float x_64 = c.y;
  c.z = (x_63 + x_64);
  x_68_phi = 0;
  while (true) {
    int x_69 = 0;
    const int x_68 = x_68_phi;
    if ((x_68 < 3)) {
    } else {
      break;
    }
    const int x_74_save = x_68;
    const float x_75 = c[x_74_save];
    if ((x_75 >= 1.0f)) {
      const float x_79 = c[x_74_save];
      const float x_80 = c[x_74_save];
      set_float3(c, x_74_save, (x_79 * x_80));
    }
    {
      x_69 = (x_68 + 1);
      x_68_phi = x_69;
    }
  }
  const float3 x_84 = normalize(abs(c));
  x_GLF_color = float4(x_84.x, x_84.y, x_84.z, 1.0f);
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
