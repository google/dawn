void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_19 : register(b1, space0) {
  uint4 x_19[1];
};
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
        const int x_155 = i;
        const float x_157 = thirty_two;
        if (((float(x_155) % round(x_157)) <= 0.01f)) {
          result = (result + 100.0f);
        }
      }
      const int x_165 = i;
      const float x_167 = limit;
      if ((float(x_165) >= x_167)) {
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
  float3 x_61 = float3(0.0f, 0.0f, 0.0f);
  int i_1 = 0;
  float j = 0.0f;
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
  const float x_77 = c.x;
  if (true) {
    x_61 = c;
  } else {
    const float3 x_82 = c;
    const float x_84 = asfloat(x_19[0].x);
    x_61 = (x_82 * x_84);
  }
  const float x_87 = x_61.y;
  c.z = (x_77 + x_87);
  i_1 = 0;
  {
    for(; (i_1 < 3); i_1 = (i_1 + 1)) {
      const float x_99 = c[i_1];
      if ((x_99 >= 1.0f)) {
        const int x_103 = i_1;
        const float x_106 = c[i_1];
        const float x_109 = c[i_1];
        set_float3(c, x_103, (x_106 * x_109));
      }
      j = 0.0f;
      while (true) {
        const float x_117 = asfloat(x_19[0].x);
        const float x_119 = asfloat(x_19[0].y);
        if ((x_117 > x_119)) {
        } else {
          break;
        }
        const float x_122 = j;
        const float x_124 = asfloat(x_19[0].x);
        if ((x_122 >= x_124)) {
          break;
        }
        j = (j + 1.0f);
      }
    }
  }
  const float3 x_134 = normalize(abs(c));
  x_GLF_color = float4(x_134.x, x_134.y, x_134.z, 1.0f);
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
