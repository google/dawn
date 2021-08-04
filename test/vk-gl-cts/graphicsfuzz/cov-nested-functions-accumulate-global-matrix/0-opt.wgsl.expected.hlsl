void set_float2(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

static float4x2 m = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b2, space0) {
  uint4 x_10[1];
};
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_16 : register(b1, space0) {
  uint4 x_16[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func0_i1_(inout int x) {
  int i = 0;
  bool x_137 = false;
  bool x_138 = false;
  bool x_138_phi = false;
  bool x_139_phi = false;
  const int x_124 = x;
  const bool x_125 = (x_124 < 1);
  x_139_phi = x_125;
  if (!(x_125)) {
    const int x_129 = x;
    const bool x_130 = (x_129 > 1);
    x_138_phi = x_130;
    if (x_130) {
      const float x_134 = asfloat(x_10[0].x);
      const uint scalar_offset = ((16u * uint(0))) / 4;
      const float x_136 = asfloat(x_12[scalar_offset / 4][scalar_offset % 4]);
      x_137 = (x_134 > x_136);
      x_138_phi = x_137;
    }
    x_138 = x_138_phi;
    x_139_phi = x_138;
  }
  if (x_139_phi) {
    return;
  }
  const float x_143 = asfloat(x_10[0].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_145 = asfloat(x_12[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_143 == x_145)) {
    i = 0;
    while (true) {
      if ((i < 2)) {
      } else {
        break;
      }
      {
        const int x_154 = x;
        const int x_155 = clamp(x_154, 0, 3);
        const int x_156 = i;
        const uint scalar_offset_2 = ((16u * uint(0))) / 4;
        const float x_158 = asfloat(x_12[scalar_offset_2 / 4][scalar_offset_2 % 4]);
        const float x_160 = m[x_155][x_156];
        set_float2(m[x_155], x_156, (x_160 + x_158));
        i = (i + 1);
      }
    }
  }
  return;
}

void func1_() {
  int param = 0;
  const float x_167 = gl_FragCoord.y;
  if ((x_167 < 0.0f)) {
    return;
  }
  param = 1;
  func0_i1_(param);
  return;
}

void main_1() {
  m = float4x2(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
  func1_();
  func1_();
  const float4x2 x_54 = m;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_56 = asint(x_16[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_59 = asint(x_16[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const int x_62 = asint(x_16[1].x);
  const int x_65 = asint(x_16[1].x);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const int x_68 = asint(x_16[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const int x_71 = asint(x_16[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  const uint scalar_offset_7 = ((16u * uint(0))) / 4;
  const int x_74 = asint(x_16[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  const uint scalar_offset_8 = ((16u * uint(0))) / 4;
  const int x_77 = asint(x_16[scalar_offset_8 / 4][scalar_offset_8 % 4]);
  const float4x2 x_83 = float4x2(float2(float(x_56), float(x_59)), float2(float(x_62), float(x_65)), float2(float(x_68), float(x_71)), float2(float(x_74), float(x_77)));
  bool tint_tmp_2 = all((x_54[0u] == x_83[0u]));
  if (tint_tmp_2) {
    tint_tmp_2 = all((x_54[1u] == x_83[1u]));
  }
  bool tint_tmp_1 = (tint_tmp_2);
  if (tint_tmp_1) {
    tint_tmp_1 = all((x_54[2u] == x_83[2u]));
  }
  bool tint_tmp = (tint_tmp_1);
  if (tint_tmp) {
    tint_tmp = all((x_54[3u] == x_83[3u]));
  }
  if ((tint_tmp)) {
    const int x_107 = asint(x_16[3].x);
    const uint scalar_offset_9 = ((16u * uint(0))) / 4;
    const int x_110 = asint(x_16[scalar_offset_9 / 4][scalar_offset_9 % 4]);
    const uint scalar_offset_10 = ((16u * uint(0))) / 4;
    const int x_113 = asint(x_16[scalar_offset_10 / 4][scalar_offset_10 % 4]);
    const int x_116 = asint(x_16[3].x);
    x_GLF_color = float4(float(x_107), float(x_110), float(x_113), float(x_116));
  } else {
    const uint scalar_offset_11 = ((16u * uint(0))) / 4;
    const int x_120 = asint(x_16[scalar_offset_11 / 4][scalar_offset_11 % 4]);
    const float x_121 = float(x_120);
    x_GLF_color = float4(x_121, x_121, x_121, x_121);
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
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
