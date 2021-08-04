cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  bool x_56 = false;
  bool x_57_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_32 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_35 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  v = float4(float(x_32), float(x_35), -621.596008301f, float(x_38));
  f = atan(trunc(v)).z;
  const float x_45 = f;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_47 = asfloat(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const bool x_49 = (x_45 > -(x_47));
  x_57_phi = x_49;
  if (x_49) {
    const float x_52 = f;
    const float x_54 = asfloat(x_9[1].x);
    x_56 = (x_52 < -(x_54));
    x_57_phi = x_56;
  }
  if (x_57_phi) {
    const int x_62 = asint(x_6[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_65 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_68 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_71 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_62), float(x_65), float(x_68), float(x_71));
  } else {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_75 = asint(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_76 = float(x_75);
    x_GLF_color = float4(x_76, x_76, x_76, x_76);
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
