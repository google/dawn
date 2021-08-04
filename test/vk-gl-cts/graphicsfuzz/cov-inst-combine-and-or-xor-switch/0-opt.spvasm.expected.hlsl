cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int count0 = 0;
  int count1 = 0;
  int i = 0;
  const int x_29 = asint(x_6[2].x);
  count0 = x_29;
  const int x_31 = asint(x_6[2].x);
  count1 = x_31;
  const int x_33 = asint(x_6[2].x);
  i = x_33;
  while (true) {
    const int x_38 = i;
    const int x_40 = asint(x_6[4].x);
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    switch(i) {
      case 0:
      case 1: {
        count0 = (count0 + 1);
        /* fallthrough */
        {
          count1 = (count1 + 1);
        }
        break;
      }
      case 2:
      case 3: {
        count1 = (count1 + 1);
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
  const int x_53 = count1;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_55 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_53 == x_55)) {
    const int x_61 = asint(x_6[3].x);
    const int x_64 = asint(x_6[2].x);
    const int x_67 = asint(x_6[2].x);
    const int x_70 = asint(x_6[3].x);
    x_GLF_color = float4(float(x_61), float(x_64), float(x_67), float(x_70));
  } else {
    const int x_74 = asint(x_6[2].x);
    const float x_75 = float(x_74);
    x_GLF_color = float4(x_75, x_75, x_75, x_75);
  }
  const int x_77 = count0;
  const int x_79 = asint(x_6[1].x);
  if ((x_77 != x_79)) {
    const int x_84 = asint(x_6[2].x);
    const float x_85 = float(x_84);
    x_GLF_color = float4(x_85, x_85, x_85, x_85);
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
