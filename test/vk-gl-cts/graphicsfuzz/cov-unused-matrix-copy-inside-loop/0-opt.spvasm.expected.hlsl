SKIP: FAILED

void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4x4 m0 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int c = 0;
  float4x4 m1 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const int x_40 = asint(x_6[1].x);
  const float x_41 = float(x_40);
  m0 = float4x4(float4(x_41, 0.0f, 0.0f, 0.0f), float4(0.0f, x_41, 0.0f, 0.0f), float4(0.0f, 0.0f, x_41, 0.0f), float4(0.0f, 0.0f, 0.0f, x_41));
  const int x_48 = asint(x_6[2].x);
  c = x_48;
  while (true) {
    const int x_53 = c;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_55 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_53 < x_55)) {
    } else {
      break;
    }
    m1 = m0;
    const int x_59 = c;
    const int x_61 = asint(x_6[3].x);
    const int x_64 = asint(x_6[2].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_66 = asfloat(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    set_float4(m1[(x_59 % x_61)], x_64, x_66);
    const int x_68 = c;
    const int x_70 = asint(x_6[3].x);
    const int x_73 = asint(x_6[2].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_75 = asfloat(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    set_float4(m0[(x_68 % x_70)], x_73, x_75);
    {
      c = (c + 1);
    }
  }
  const float4x4 x_79 = m0;
  const int x_81 = asint(x_6[1].x);
  const int x_84 = asint(x_6[2].x);
  const int x_87 = asint(x_6[1].x);
  const int x_90 = asint(x_6[1].x);
  const int x_93 = asint(x_6[1].x);
  const int x_96 = asint(x_6[2].x);
  const int x_99 = asint(x_6[1].x);
  const int x_102 = asint(x_6[1].x);
  const int x_105 = asint(x_6[1].x);
  const int x_108 = asint(x_6[2].x);
  const int x_111 = asint(x_6[1].x);
  const int x_114 = asint(x_6[1].x);
  const int x_117 = asint(x_6[1].x);
  const int x_120 = asint(x_6[2].x);
  const int x_123 = asint(x_6[1].x);
  const int x_126 = asint(x_6[1].x);
  const float4x4 x_132 = float4x4(float4(float(x_81), float(x_84), float(x_87), float(x_90)), float4(float(x_93), float(x_96), float(x_99), float(x_102)), float4(float(x_105), float(x_108), float(x_111), float(x_114)), float4(float(x_117), float(x_120), float(x_123), float(x_126)));
  if ((((all((x_79[0u] == x_132[0u])) & all((x_79[1u] == x_132[1u]))) & all((x_79[2u] == x_132[2u]))) & all((x_79[3u] == x_132[3u])))) {
    const int x_156 = asint(x_6[2].x);
    const int x_159 = asint(x_6[1].x);
    const int x_162 = asint(x_6[1].x);
    const int x_165 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_156), float(x_159), float(x_162), float(x_165));
  } else {
    const int x_169 = asint(x_6[1].x);
    const float x_170 = float(x_169);
    x_GLF_color = float4(x_170, x_170, x_170, x_170);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_4 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_4;
}
C:\src\tint\test\Shader@0x00000204988F8F80(36,20-30): warning X3556: integer modulus may be much slower, try using uints if possible.
C:\src\tint\test\Shader@0x00000204988F8F80(36,16-32): error X3500: array reference cannot be used as an l-value; not natively addressable
C:\src\tint\test\Shader@0x00000204988F8F80(22,3-14): error X3511: forced to unroll loop, but unrolling failed.

