void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

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
        const int x_122 = i;
        const float x_124 = thirty_two;
        if (((float(x_122) % round(x_124)) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      const int x_132 = i;
      const float x_134 = limit;
      if ((float(x_132) >= x_134)) {
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
  c = float3(7.0f, 8.0f, 9.0f);
  const float x_56 = asfloat(x_13[0].x);
  thirty_two_1 = round((x_56 / 8.0f));
  const float x_60 = gl_FragCoord.x;
  param = x_60;
  param_1 = thirty_two_1;
  const float x_62 = compute_value_f1_f1_(param, param_1);
  c.x = x_62;
  const float x_65 = gl_FragCoord.y;
  param_2 = x_65;
  param_3 = thirty_two_1;
  const float x_67 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_67;
  const float x_70 = c.x;
  const float x_72 = c.y;
  c.z = (x_70 + x_72);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      const float x_84 = c[i_1];
      if ((x_84 >= 1.0f)) {
        const int x_88 = i_1;
        const float x_91 = c[i_1];
        const float x_94 = c[i_1];
        set_float3(c, x_88, (x_91 * x_94));
      }
    }
  }
  const float3 x_101 = normalize(abs(c));
  x_GLF_color = float4(x_101.x, x_101.y, x_101.z, 1.0f);
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
