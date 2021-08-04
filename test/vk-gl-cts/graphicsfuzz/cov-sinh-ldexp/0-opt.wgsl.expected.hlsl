cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v1 = float2(0.0f, 0.0f);
  int2 v2 = int2(0, 0);
  float2 v3 = float2(0.0f, 0.0f);
  bool x_66 = false;
  bool x_67_phi = false;
  const float x_41 = asfloat(x_6[2].x);
  const float x_43 = asfloat(x_6[3].x);
  v1 = sinh(float2(x_41, x_43));
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_47 = asint(x_9[scalar_offset / 4][scalar_offset % 4]);
  v2 = int2(x_47, -3000);
  v3 = ldexp(v1, v2);
  const float x_53 = v3.y;
  x_GLF_color = float4(x_53, x_53, x_53, x_53);
  const float x_56 = v3.x;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_58 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const bool x_59 = (x_56 > x_58);
  x_67_phi = x_59;
  if (x_59) {
    const float x_63 = v3.x;
    const float x_65 = asfloat(x_6[1].x);
    x_66 = (x_63 < x_65);
    x_67_phi = x_66;
  }
  if (x_67_phi) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_72 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_75 = asint(x_9[1].x);
    const int x_78 = asint(x_9[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_81 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_72), float(x_75), float(x_78), float(x_81));
  } else {
    const int x_85 = asint(x_9[1].x);
    const float x_86 = float(x_85);
    x_GLF_color = float4(x_86, x_86, x_86, x_86);
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
