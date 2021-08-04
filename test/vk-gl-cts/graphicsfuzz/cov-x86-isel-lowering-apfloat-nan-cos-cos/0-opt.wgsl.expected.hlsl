cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};

void main_1() {
  float2 v1 = float2(0.0f, 0.0f);
  bool x_54 = false;
  bool x_55_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_35 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  v1 = cos(cos(asfloat(int2(-1, x_35))));
  const float x_41 = v1.x;
  x_GLF_color = float4(x_41, x_41, x_41, x_41);
  const float x_44 = v1.y;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_46 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const bool x_47 = (x_44 > x_46);
  x_55_phi = x_47;
  if (x_47) {
    const float x_51 = v1.y;
    const float x_53 = asfloat(x_8[1].x);
    x_54 = (x_51 < x_53);
    x_55_phi = x_54;
  }
  if (x_55_phi) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_60 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_63 = asint(x_6[1].x);
    const int x_66 = asint(x_6[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_69 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_60), float(x_63), float(x_66), float(x_69));
  } else {
    const int x_73 = asint(x_6[1].x);
    const float x_74 = float(x_73);
    x_GLF_color = float4(x_74, x_74, x_74, x_74);
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
