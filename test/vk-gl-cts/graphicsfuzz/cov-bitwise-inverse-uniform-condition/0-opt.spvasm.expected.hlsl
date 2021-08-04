cbuffer cbuffer_x_6 : register(b2, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int x_32 = 0;
  const float x_34 = asfloat(x_6[0].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  if ((x_34 < x_36)) {
    const int x_42 = asint(x_10[1].x);
    x_32 = x_42;
  } else {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_44 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_32 = x_44;
  }
  a = ~((x_32 | 1));
  const int x_48 = a;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_50 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if ((x_48 == ~(x_50))) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_60 = asint(x_10[1].x);
    const int x_63 = asint(x_10[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_66 = asint(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(float(x_57), float(x_60), float(x_63), float(x_66));
  } else {
    const int x_70 = asint(x_10[1].x);
    const float x_71 = float(x_70);
    x_GLF_color = float4(x_71, x_71, x_71, x_71);
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
