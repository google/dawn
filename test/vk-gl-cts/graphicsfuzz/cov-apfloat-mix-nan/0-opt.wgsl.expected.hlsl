cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[10];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float nan = 0.0f;
  float4 undefined = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_83 = false;
  bool x_84_phi = false;
  nan = asfloat(-1);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_43 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  const int x_46 = asint(x_7[1].x);
  const int x_49 = asint(x_7[2].x);
  const int x_52 = asint(x_7[3].x);
  const int x_56 = asint(x_7[4].x);
  const int x_59 = asint(x_7[5].x);
  const int x_62 = asint(x_7[6].x);
  const int x_65 = asint(x_7[7].x);
  const float x_68 = nan;
  undefined = lerp(float4(float(x_43), float(x_46), float(x_49), float(x_52)), float4(float(x_56), float(x_59), float(x_62), float(x_65)), float4(x_68, x_68, x_68, x_68));
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_72 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const int x_74 = asint(x_7[9].x);
  const bool x_75 = (x_72 == x_74);
  x_84_phi = x_75;
  if (!(x_75)) {
    const float x_80 = undefined.x;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_82 = asfloat(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_83 = (x_80 > x_82);
    x_84_phi = x_83;
  }
  if (x_84_phi) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_89 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_92 = asint(x_7[8].x);
    const int x_95 = asint(x_7[8].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_98 = asint(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(float(x_89), float(x_92), float(x_95), float(x_98));
  } else {
    const int x_102 = asint(x_7[8].x);
    const float x_103 = float(x_102);
    x_GLF_color = float4(x_103, x_103, x_103, x_103);
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
