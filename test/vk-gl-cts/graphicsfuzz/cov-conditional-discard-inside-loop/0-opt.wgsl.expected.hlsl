static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_9 : register(b2, space0) {
  uint4 x_9[1];
};
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[1];
};

void main_1() {
  int a = 0;
  int i = 0;
  a = 1;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const int x_41 = asint(x_6[1].x);
  const int x_44 = asint(x_6[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_47 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_38), float(x_41), float(x_44), float(x_47));
  const int x_51 = asint(x_6[1].x);
  i = x_51;
  while (true) {
    const int x_56 = i;
    const int x_58 = asint(x_6[2].x);
    if ((x_56 < x_58)) {
    } else {
      break;
    }
    const int x_61 = a;
    a = (x_61 + 1);
    if ((x_61 > 3)) {
      break;
    }
    const float x_67 = asfloat(x_9[0].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_69 = asfloat(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_67 > x_69)) {
      discard;
    }
    {
      i = (i + 1);
    }
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
