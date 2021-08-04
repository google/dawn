cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  a = 1;
  while (true) {
    const int x_29 = a;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_31 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_29 >= x_31)) {
      break;
    }
    if (true) {
      discard;
    }
    a = (a + 1);
    {
      if ((a != 1)) {
      } else {
        break;
      }
    }
  }
  if ((a == 1)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_50 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_53 = asint(x_6[1].x);
    x_GLF_color = float4(1.0f, float(x_47), float(x_50), float(x_53));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_58 = float(x_57);
    x_GLF_color = float4(x_58, x_58, x_58, x_58);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
