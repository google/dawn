cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const int x_26 = asint(x_6[2].x);
  a = x_26;
  const int x_28 = asint(x_6[2].x);
  i = x_28;
  while (true) {
    const int x_33 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_35 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_33 < x_35)) {
    } else {
      break;
    }
    switch(i) {
      case 0:
      case -1: {
        const int x_42 = asint(x_6[1].x);
        a = x_42;
        break;
      }
      default: {
        break;
      }
    }
    {
      i = (i + 1);
    }
  }
  const int x_45 = a;
  const int x_47 = asint(x_6[1].x);
  if ((x_45 == x_47)) {
    const int x_53 = asint(x_6[1].x);
    const int x_56 = asint(x_6[2].x);
    const int x_59 = asint(x_6[2].x);
    const int x_62 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_53), float(x_56), float(x_59), float(x_62));
  } else {
    const int x_66 = asint(x_6[2].x);
    const float x_67 = float(x_66);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
