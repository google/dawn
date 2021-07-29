SKIP: FAILED

void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float3x4 m = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 indexable = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_44 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = x_44;
  const float x_46 = asfloat(x_9[2].x);
  v = float4(x_46, x_46, x_46, x_46);
  const float x_49 = asfloat(x_9[3].x);
  m = float3x4(float4(x_49, 0.0f, 0.0f, 0.0f), float4(0.0f, x_49, 0.0f, 0.0f), float4(0.0f, 0.0f, x_49, 0.0f));
  const int x_54 = a;
  const int x_55 = a;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_57 = asfloat(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  set_float4(m[x_54], x_55, x_57);
  const int x_59 = a;
  const float3x4 x_60 = m;
  const int x_78 = a;
  const int x_79 = a;
  indexable = float4x4(float4(x_60[0u].x, x_60[0u].y, x_60[0u].z, x_60[0u].w), float4(x_60[1u].x, x_60[1u].y, x_60[1u].z, x_60[1u].w), float4(x_60[2u].x, x_60[2u].y, x_60[2u].z, x_60[2u].w), float4(0.0f, 0.0f, 0.0f, 1.0f));
  const float x_81 = indexable[x_78][x_79];
  const float x_83 = v[x_59];
  set_float4(v, x_59, (x_83 + x_81));
  const float x_87 = v.y;
  const float x_89 = asfloat(x_9[1].x);
  if ((x_87 == x_89)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_95 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_98 = asint(x_6[1].x);
    const int x_101 = asint(x_6[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_104 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_95), float(x_98), float(x_101), float(x_104));
  } else {
    const int x_108 = asint(x_6[1].x);
    const float x_109 = float(x_108);
    x_GLF_color = float4(x_109, x_109, x_109, x_109);
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
C:\src\tint\test\Shader@0x000001FE2E0FB130(29,14-20): error X3500: array reference cannot be used as an l-value; not natively addressable

