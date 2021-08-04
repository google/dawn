cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b2, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

void main_1() {
  uint a = 0u;
  float b = 0.0f;
  uint c = 0u;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const uint x_38 = x_6[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  a = (x_38 >> uint(x_40));
  b = asfloat(a);
  c = asuint(b);
  const uint x_47 = c;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const uint x_49 = x_6[scalar_offset_2 / 4][scalar_offset_2 % 4];
  if ((x_47 == x_49)) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_55 = asint(x_12[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_58 = asint(x_12[1].x);
    const int x_61 = asint(x_12[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_64 = asint(x_12[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(float(x_55), float(x_58), float(x_61), float(x_64));
  } else {
    const float x_67 = b;
    x_GLF_color = float4(x_67, x_67, x_67, x_67);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
