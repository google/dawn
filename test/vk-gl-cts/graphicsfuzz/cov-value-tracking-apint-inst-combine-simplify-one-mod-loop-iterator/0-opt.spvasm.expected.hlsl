cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const int x_27 = asint(x_6[2].x);
  a = x_27;
  const int x_29 = asint(x_6[3].x);
  i = x_29;
  while (true) {
    const int x_34 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_36 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_34 < x_36)) {
    } else {
      break;
    }
    const int x_39 = i;
    const int x_42 = asint(x_6[2].x);
    if (((1 % x_39) == x_42)) {
      {
        i = (i + 1);
      }
      continue;
    }
    a = (a + 1);
    {
      i = (i + 1);
    }
  }
  const int x_50 = a;
  const int x_52 = asint(x_6[1].x);
  if ((x_50 == x_52)) {
    const int x_58 = asint(x_6[3].x);
    const int x_61 = asint(x_6[2].x);
    const int x_64 = asint(x_6[2].x);
    const int x_67 = asint(x_6[3].x);
    x_GLF_color = float4(float(x_58), float(x_61), float(x_64), float(x_67));
  } else {
    const int x_71 = asint(x_6[2].x);
    const float x_72 = float(x_71);
    x_GLF_color = float4(x_72, x_72, x_72, x_72);
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
