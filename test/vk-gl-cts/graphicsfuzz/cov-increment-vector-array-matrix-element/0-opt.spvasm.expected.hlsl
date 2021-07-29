SKIP: FAILED

void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float3x3 m = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int a = 0;
  float3 arr[2] = (float3[2])0;
  float3 v = float3(0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_45 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_46 = float(x_45);
  m = float3x3(float3(x_46, 0.0f, 0.0f), float3(0.0f, x_46, 0.0f), float3(0.0f, 0.0f, x_46));
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_52 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  a = x_52;
  const int x_53 = a;
  const int x_54 = a;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_56 = asfloat(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  set_float3(m[x_53], x_54, x_56);
  const float3 x_59 = m[1];
  const float3 x_61 = m[1];
  const float3 tint_symbol_4[2] = {x_59, x_61};
  arr = tint_symbol_4;
  const float x_64 = asfloat(x_9[1].x);
  v = float3(x_64, x_64, x_64);
  const float3 x_68 = arr[a];
  v = (v + x_68);
  const float3 x_71 = v;
  const int x_73 = asint(x_6[1].x);
  const int x_76 = asint(x_6[2].x);
  const int x_79 = asint(x_6[1].x);
  if (all((x_71 == float3(float(x_73), float(x_76), float(x_79))))) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_88 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_91 = asint(x_6[3].x);
    const int x_94 = asint(x_6[3].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_97 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(float(x_88), float(x_91), float(x_94), float(x_97));
  } else {
    const int x_101 = asint(x_6[3].x);
    const float x_102 = float(x_101);
    x_GLF_color = float4(x_102, x_102, x_102, x_102);
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
  const tint_symbol tint_symbol_5 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_5;
}
C:\src\tint\test\Shader@0x000001C5FF133840(29,14-20): error X3500: array reference cannot be used as an l-value; not natively addressable

