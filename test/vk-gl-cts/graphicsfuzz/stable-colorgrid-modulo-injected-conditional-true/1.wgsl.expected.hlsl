void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_20 : register(b1, space0) {
  uint4 x_20[1];
};

float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float result = 0.0f;
  int i = 0;
  result = -0.5f;
  i = 1;
  {
    for(; (i < 800); i = (i + 1)) {
      if (((i % 32) == 0)) {
        result = (result + 0.400000006f);
      } else {
        const int x_136 = i;
        const float x_138 = thirty_two;
        if (((float(x_136) % round(x_138)) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      const int x_146 = i;
      const float x_148 = limit;
      if ((float(x_146) >= x_148)) {
        return result;
      }
    }
  }
  return result;
}

void main_1() {
  float3 c = float3(0.0f, 0.0f, 0.0f);
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int i_1 = 0;
  float3 x_58 = float3(0.0f, 0.0f, 0.0f);
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_60 = asfloat(x_13[0].x);
  thirty_two_1 = round((x_60 / 8.0f));
  const float x_64 = gl_FragCoord.x;
  param = x_64;
  param_1 = thirty_two_1;
  const float x_66 = compute_value_f1_f1_(param, param_1);
  c.x = x_66;
  const float x_69 = gl_FragCoord.y;
  param_2 = x_69;
  param_3 = thirty_two_1;
  const float x_71 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_71;
  const float x_74 = c.x;
  const float x_76 = c.y;
  c.z = (x_74 + x_76);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      const float x_88 = c[i_1];
      if ((x_88 >= 1.0f)) {
        const int x_92 = i_1;
        const float x_95 = c[i_1];
        const float x_98 = c[i_1];
        set_float3(c, x_92, (x_95 * x_98));
      }
    }
  }
  const float x_104 = asfloat(x_20[0].x);
  const float x_106 = asfloat(x_20[0].y);
  if ((x_104 < x_106)) {
    x_58 = abs(c);
  } else {
    x_58 = c;
  }
  const float3 x_115 = normalize(x_58);
  x_GLF_color = float4(x_115.x, x_115.y, x_115.z, 1.0f);
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
