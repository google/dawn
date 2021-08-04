cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[12];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_15 : register(b0, space0) {
  uint4 x_15[1];
};

void main_1() {
  float3x4 m0 = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x4 m1 = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3 undefined = float3(0.0f, 0.0f, 0.0f);
  float3 defined = float3(0.0f, 0.0f, 0.0f);
  float4 v0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const int x_17 = asint(x_6[4].x);
  const int x_18 = asint(x_6[5].x);
  const int x_19 = asint(x_6[6].x);
  const int x_20 = asint(x_6[10].x);
  const int x_21 = asint(x_6[7].x);
  const int x_22 = asint(x_6[8].x);
  const int x_23 = asint(x_6[11].x);
  const int x_24 = asint(x_6[1].x);
  const int x_25 = asint(x_6[2].x);
  const int x_26 = asint(x_6[3].x);
  m0 = float3x4(float4(float(x_17), float(x_18), float(x_19), 4.0f), float4(float(x_20), float(x_21), float(x_22), 8.0f), float4(float(x_23), float(x_24), float(x_25), float(x_26)));
  const int x_27 = asint(x_6[4].x);
  const float x_104 = float(x_27);
  m1 = float3x4(float4(x_104, 0.0f, 0.0f, 0.0f), float4(0.0f, x_104, 0.0f, 0.0f), float4(0.0f, 0.0f, x_104, 0.0f));
  undefined = ldexp(float3(1.0f, 1.0f, 1.0f), int3(500, 500, 500));
  const int x_28 = asint(x_6[4].x);
  const float x_111 = float(x_28);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  defined = ldexp(float3(x_111, x_111, x_111), int3(x_29, x_29, x_29));
  v0 = mul(undefined, m0);
  v1 = mul(undefined, m1);
  v2 = mul(defined, m0);
  v3 = mul(defined, m1);
  const float x_129 = v2.x;
  const float x_131 = v3.x;
  if ((x_129 > x_131)) {
    const int x_30 = asint(x_6[4].x);
    const int x_31 = asint(x_6[9].x);
    const int x_32 = asint(x_6[9].x);
    const int x_33 = asint(x_6[4].x);
    x_GLF_color = float4(float(x_30), float(x_31), float(x_32), float(x_33));
  } else {
    const int x_34 = asint(x_6[9].x);
    const float x_146 = float(x_34);
    x_GLF_color = float4(x_146, x_146, x_146, x_146);
  }
  const float x_149 = v0.x;
  const float x_151 = v1.x;
  if ((x_149 < x_151)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_156 = asfloat(x_15[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color.y = x_156;
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
