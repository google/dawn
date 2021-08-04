cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_f1_(inout float f) {
  int a = 0;
  int b = 0;
  int i = 0;
  const int x_60 = asint(x_8[1].x);
  a = x_60;
  const int x_62 = asint(x_8[2].x);
  b = x_62;
  const int x_64 = asint(x_8[2].x);
  i = x_64;
  while (true) {
    const int x_69 = i;
    const int x_71 = asint(x_8[4].x);
    if ((x_69 < x_71)) {
    } else {
      break;
    }
    const int x_74 = a;
    const int x_76 = asint(x_8[3].x);
    if ((x_74 > x_76)) {
      break;
    }
    const float x_80 = f;
    const int x_84 = asint(x_8[1].x);
    a = (((int(x_80) - 1) - x_84) + i);
    b = (b + 1);
    {
      i = (i + 1);
    }
  }
  const int x_92 = b;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_94 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
  if ((x_92 == x_94)) {
    const int x_100 = asint(x_8[1].x);
    return x_100;
  } else {
    const int x_102 = asint(x_8[2].x);
    return x_102;
  }
  return 0;
}

void main_1() {
  float param = 0.0f;
  param = 0.699999988f;
  const int x_34 = func_f1_(param);
  const int x_36 = asint(x_8[1].x);
  if ((x_34 == x_36)) {
    const int x_42 = asint(x_8[1].x);
    const int x_45 = asint(x_8[2].x);
    const int x_48 = asint(x_8[2].x);
    const int x_51 = asint(x_8[1].x);
    x_GLF_color = float4(float(x_42), float(x_45), float(x_48), float(x_51));
  } else {
    const int x_55 = asint(x_8[2].x);
    const float x_56 = float(x_55);
    x_GLF_color = float4(x_56, x_56, x_56, x_56);
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
