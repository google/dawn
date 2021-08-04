static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[3];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int i = 0;
  x_GLF_global_loop_count = 0;
  const float x_36 = asfloat(x_7[1].x);
  f = x_36;
  const int x_38 = asint(x_10[1].x);
  i = x_38;
  while (true) {
    const int x_43 = i;
    const int x_45 = asint(x_10[2].x);
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    const float x_48 = f;
    const float x_50 = asfloat(x_7[1].x);
    if ((x_48 > x_50)) {
    }
    f = 1.0f;
    const float x_55 = asfloat(x_7[2].x);
    f = ((1.0f - clamp(x_55, 1.0f, f)) + float(i));
    {
      i = (i + 1);
    }
  }
  const float x_64 = f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_66 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  if ((x_64 == x_66)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_72 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_75 = asint(x_10[1].x);
    const int x_78 = asint(x_10[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_81 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_72), float(x_75), float(x_78), float(x_81));
  } else {
    const int x_85 = asint(x_10[1].x);
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
