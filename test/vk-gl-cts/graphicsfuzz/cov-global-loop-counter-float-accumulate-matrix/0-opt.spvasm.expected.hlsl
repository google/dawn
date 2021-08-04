void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[18];
};
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[4];
};
cbuffer cbuffer_x_15 : register(b2, space0) {
  uint4 x_15[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4x4 m = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  int a = 0;
  int b = 0;
  float zero = 0.0f;
  x_GLF_global_loop_count = 0;
  const float x_62 = asfloat(x_7[1].x);
  const float x_64 = asfloat(x_7[2].x);
  const float x_66 = asfloat(x_7[3].x);
  const float x_68 = asfloat(x_7[4].x);
  const float x_70 = asfloat(x_7[5].x);
  const float x_72 = asfloat(x_7[6].x);
  const float x_74 = asfloat(x_7[7].x);
  const float x_76 = asfloat(x_7[8].x);
  const float x_78 = asfloat(x_7[9].x);
  const float x_80 = asfloat(x_7[10].x);
  const float x_82 = asfloat(x_7[11].x);
  const float x_84 = asfloat(x_7[12].x);
  const float x_86 = asfloat(x_7[13].x);
  const float x_88 = asfloat(x_7[14].x);
  const float x_90 = asfloat(x_7[15].x);
  const float x_92 = asfloat(x_7[16].x);
  m = float4x4(float4(x_62, x_64, x_66, x_68), float4(x_70, x_72, x_74, x_76), float4(x_78, x_80, x_82, x_84), float4(x_86, x_88, x_90, x_92));
  const float x_99 = asfloat(x_7[1].x);
  const float x_101 = asfloat(x_7[2].x);
  const float x_103 = asfloat(x_7[3].x);
  const float x_105 = asfloat(x_7[4].x);
  v = float4(x_99, x_101, x_103, x_105);
  const float x_108 = asfloat(x_7[1].x);
  f = x_108;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_110 = asint(x_12[scalar_offset / 4][scalar_offset % 4]);
  a = x_110;
  {
    for(; (x_GLF_global_loop_count < 10); a = (a + 1)) {
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      const int x_121 = clamp(a, 0, 3);
      const float x_123 = asfloat(x_7[1].x);
      const float x_125 = v[x_121];
      set_float4(v, x_121, (x_125 + x_123));
      const int x_129 = asint(x_12[2].x);
      b = x_129;
      {
        for(; (x_GLF_global_loop_count < 10); b = (b - 1)) {
          x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
          const float x_142 = v[clamp(b, 0, 3)];
          const float x_147 = m[clamp(b, 0, 3)][a];
          f = (f + (x_142 * x_147));
        }
      }
      const int x_153 = a;
      const float x_156 = asfloat(x_7[1].x);
      set_float4(m[1], clamp(x_153, 0, 3), x_156);
      const int x_159 = asint(x_15[0].x);
      const uint scalar_offset_1 = ((16u * uint(0))) / 4;
      const int x_161 = asint(x_12[scalar_offset_1 / 4][scalar_offset_1 % 4]);
      if ((x_159 == x_161)) {
        discard;
      }
      const int x_166 = asint(x_15[0].x);
      const int x_168 = asint(x_12[1].x);
      if ((x_166 == x_168)) {
        discard;
      }
    }
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_175 = asfloat(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  zero = x_175;
  const float x_176 = f;
  const float x_178 = asfloat(x_7[17].x);
  if (!((x_176 == x_178))) {
    const float x_183 = asfloat(x_7[1].x);
    zero = x_183;
  }
  const float x_184 = f;
  const float x_185 = zero;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_187 = asint(x_12[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  x_GLF_color = float4(x_184, x_185, float(x_187), f);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
