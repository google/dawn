cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int b = 0;
  int c = 0;
  bool x_76 = false;
  bool x_83 = false;
  bool x_77_phi = false;
  bool x_84_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_31 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = x_31;
  const int x_33 = asint(x_6[2].x);
  b = x_33;
  c = 1;
  while (true) {
    const int x_38 = b;
    const int x_40 = asint(x_6[4].x);
    if (((x_38 < x_40) & (a < 10))) {
    } else {
      break;
    }
    if ((c > 5)) {
      break;
    }
    a = (a + 1);
    c = (c + 1);
    b = (b + 1);
  }
  while (true) {
    const int x_60 = a;
    const int x_62 = asint(x_6[1].x);
    if ((x_60 < x_62)) {
    } else {
      break;
    }
    {
      a = (a + 1);
    }
  }
  const int x_67 = a;
  const int x_69 = asint(x_6[1].x);
  const bool x_70 = (x_67 == x_69);
  x_77_phi = x_70;
  if (x_70) {
    const int x_73 = b;
    const int x_75 = asint(x_6[3].x);
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  const bool x_77 = x_77_phi;
  x_84_phi = x_77;
  if (x_77) {
    const int x_80 = c;
    const int x_82 = asint(x_6[3].x);
    x_83 = (x_80 == x_82);
    x_84_phi = x_83;
  }
  if (x_84_phi) {
    const int x_89 = asint(x_6[2].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_92 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_95 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_98 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_89), float(x_92), float(x_95), float(x_98));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_102 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_103 = float(x_102);
    x_GLF_color = float4(x_103, x_103, x_103, x_103);
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
