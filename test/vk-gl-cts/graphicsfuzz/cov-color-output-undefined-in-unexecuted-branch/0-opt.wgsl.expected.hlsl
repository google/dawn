float4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b2, space0) {
  uint4 x_12[4];
};
cbuffer cbuffer_x_14 : register(b3, space0) {
  uint4 x_14[1];
};
cbuffer cbuffer_x_16 : register(b0, space0) {
  uint4 x_16[1];
};

void func0_() {
  float4 tmp = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_112 = gl_FragCoord.x;
  const float x_114 = asfloat(x_8[1].x);
  if ((x_112 > x_114)) {
    tmp = x_GLF_color;
  }
  x_GLF_color = tmp;
  return;
}

int func1_() {
  int a = 0;
  const int x_122 = asint(x_12[1].x);
  a = x_122;
  while (true) {
    const int x_127 = a;
    const int x_129 = asint(x_12[3].x);
    if ((x_127 < x_129)) {
    } else {
      break;
    }
    const int x_133 = asint(x_14[0].x);
    const int x_135 = asint(x_12[1].x);
    if ((x_133 > x_135)) {
      func0_();
      const int x_142 = asint(x_12[3].x);
      a = x_142;
    } else {
      func0_();
    }
  }
  return a;
}

void main_1() {
  int a_1 = 0;
  int i = 0;
  int j = 0;
  const float x_56 = gl_FragCoord.x;
  const float x_58 = asfloat(x_8[1].x);
  if ((x_56 > x_58)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_64 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
    const float x_66 = asfloat(x_8[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_68 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_70 = asfloat(x_8[2].x);
    x_GLF_color = float4(x_64, x_66, x_68, x_70);
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const uint x_73 = x_16[scalar_offset_2 / 4][scalar_offset_2 % 4];
    x_GLF_color = tint_unpack4x8snorm(x_73);
  }
  const int x_76 = asint(x_12[2].x);
  a_1 = x_76;
  i = 0;
  {
    for(; (i < 5); i = (i + 1)) {
      j = 0;
      {
        for(; (j < 2); j = (j + 1)) {
          const int x_91 = func1_();
          a_1 = (a_1 + x_91);
        }
      }
    }
  }
  const int x_98 = a_1;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_100 = asint(x_12[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  if ((x_98 == x_100)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_105 = asfloat(x_8[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_107 = x_GLF_color.z;
    x_GLF_color.z = (x_107 - x_105);
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
  const main_out tint_symbol_7 = {x_GLF_color};
  return tint_symbol_7;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
