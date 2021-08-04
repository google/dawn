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
        const int x_147 = i;
        const float x_149 = thirty_two;
        if (((float(x_147) - (round(x_149) * floor((float(x_147) / round(x_149))))) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      const int x_157 = i;
      const float x_159 = limit;
      if ((float(x_157) >= x_159)) {
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
  const float x_63 = asfloat(x_13[0].x);
  thirty_two_1 = round((x_63 / 8.0f));
  const float x_67 = gl_FragCoord.x;
  param = x_67;
  param_1 = thirty_two_1;
  const float x_69 = compute_value_f1_f1_(param, param_1);
  c.x = x_69;
  const float x_72 = gl_FragCoord.y;
  param_2 = x_72;
  param_3 = thirty_two_1;
  const float x_74 = compute_value_f1_f1_(param_2, param_3);
  c.y = x_74;
  const float3 x_76 = c;
  const float3 x_79 = c;
  const float4x2 x_87 = float4x2(float2(x_79.x, x_79.y), float2(x_79.z, 1.0f), float2(1.0f, 0.0f), float2(1.0f, 0.0f));
  c.z = (mul(float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f)), x_76).x + float3(x_87[0u].x, x_87[0u].y, x_87[1u].x).y);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      const float x_104 = c[i_1];
      if ((x_104 >= 1.0f)) {
        const int x_108 = i_1;
        const float x_111 = c[i_1];
        const float x_114 = c[i_1];
        set_float3(c, x_108, (x_111 * x_114));
        const float x_118 = gl_FragCoord.y;
        if ((x_118 < 0.0f)) {
          break;
        }
      }
    }
  }
  const float3 x_126 = normalize(abs(c));
  x_GLF_color = float4(x_126.x, x_126.y, x_126.z, 1.0f);
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
