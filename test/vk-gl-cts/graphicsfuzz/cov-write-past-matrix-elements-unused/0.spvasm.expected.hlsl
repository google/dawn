SKIP: FAILED

void set_float2(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float3x2 m32 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float sums[3] = (float[3])0;
  int x_52_phi = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  m32 = float3x2(float2(x_40, 0.0f), float2(0.0f, x_40), float2(0.0f, 0.0f));
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_45 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_45 == 1)) {
    set_float2(m32[3], x_45, x_40);
  }
  const float tint_symbol_4[3] = {x_40, x_40, x_40};
  sums = tint_symbol_4;
  x_52_phi = x_45;
  while (true) {
    int x_53 = 0;
    const int x_52 = x_52_phi;
    const int x_56 = asint(x_8[2].x);
    if ((x_52 < x_56)) {
    } else {
      break;
    }
    {
      const float x_60 = m32[x_52][x_45];
      const int x_61_save = x_56;
      const float x_62 = sums[x_61_save];
      sums[x_61_save] = (x_62 + x_60);
      x_53 = (x_52 + 1);
      x_52_phi = x_53;
    }
  }
  const float x_65 = sums[x_45];
  const float x_67 = asfloat(x_6[1].x);
  const int x_69 = asint(x_8[1].x);
  const float x_71 = sums[x_69];
  x_GLF_color = float4(x_65, x_67, x_67, x_71);
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
  const tint_symbol tint_symbol_5 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_5;
}
C:\src\tint\test\Shader@0x0000029DDF1A00E0(23,16-21): error X3504: array index out of bounds

