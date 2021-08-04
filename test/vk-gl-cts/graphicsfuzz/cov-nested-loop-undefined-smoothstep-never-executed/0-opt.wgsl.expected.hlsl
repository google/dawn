void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int a = 0;
  int c = 0;
  const float x_41 = asfloat(x_6[1].x);
  v0 = float4(x_41, x_41, x_41, x_41);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_44 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  v1 = float4(x_44, x_44, x_44, x_44);
  const int x_47 = asint(x_10[1].x);
  a = x_47;
  while (true) {
    const int x_52 = a;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_54 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_52 < x_54)) {
    } else {
      break;
    }
    const int x_58 = asint(x_10[3].x);
    c = x_58;
    while (true) {
      const int x_63 = c;
      const int x_65 = asint(x_10[2].x);
      if ((x_63 < x_65)) {
      } else {
        break;
      }
      const int x_69 = clamp(c, 0, 3);
      const float x_71 = asfloat(x_6[1].x);
      const float x_73 = v0[x_69];
      set_float4(v0, x_69, (x_73 - x_71));
      const int x_77 = asint(x_10[1].x);
      const int x_79 = asint(x_10[3].x);
      if ((x_77 == x_79)) {
        const int x_83 = a;
        const float x_85 = asfloat(x_6[1].x);
        const float x_87 = asfloat(x_6[1].x);
        const float x_89 = asfloat(x_6[1].x);
        set_float4(v1, x_83, smoothstep(float4(x_85, x_87, x_89, 3.0f), float4(1.0f, 1.0f, 1.0f, 1.0f), v0)[a]);
      }
      {
        c = (c + 1);
      }
    }
    {
      a = (a + 1);
    }
  }
  const float x_101 = v1.x;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_103 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if ((x_101 == x_103)) {
    const int x_109 = asint(x_10[1].x);
    const int x_112 = asint(x_10[3].x);
    const int x_115 = asint(x_10[3].x);
    const int x_118 = asint(x_10[1].x);
    x_GLF_color = float4(float(x_109), float(x_112), float(x_115), float(x_118));
  } else {
    const int x_122 = asint(x_10[3].x);
    const float x_123 = float(x_122);
    x_GLF_color = float4(x_123, x_123, x_123, x_123);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
