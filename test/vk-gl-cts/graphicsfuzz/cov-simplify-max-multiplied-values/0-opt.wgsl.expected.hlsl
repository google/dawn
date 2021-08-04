cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int A[4] = (int[4])0;
  bool x_77 = false;
  bool x_87 = false;
  bool x_97 = false;
  bool x_78_phi = false;
  bool x_88_phi = false;
  bool x_98_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_33 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  i = x_33;
  while (true) {
    const int x_38 = i;
    const int x_40 = asint(x_6[4].x);
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    const int x_43 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_45 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    A[x_43] = x_45;
    const int x_47 = i;
    const int x_50 = asint(x_6[3].x);
    const int x_54 = asint(x_6[1].x);
    if ((max((2 * x_47), (2 * x_50)) == x_54)) {
      A[i] = 1;
    }
    {
      i = (i + 1);
    }
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_63 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int x_65 = A[x_63];
  const int x_67 = asint(x_6[3].x);
  const bool x_68 = (x_65 == x_67);
  x_78_phi = x_68;
  if (x_68) {
    const int x_72 = asint(x_6[3].x);
    const int x_74 = A[x_72];
    const int x_76 = asint(x_6[3].x);
    x_77 = (x_74 == x_76);
    x_78_phi = x_77;
  }
  const bool x_78 = x_78_phi;
  x_88_phi = x_78;
  if (x_78) {
    const int x_82 = asint(x_6[1].x);
    const int x_84 = A[x_82];
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_86 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_87 = (x_84 == x_86);
    x_88_phi = x_87;
  }
  const bool x_88 = x_88_phi;
  x_98_phi = x_88;
  if (x_88) {
    const int x_92 = asint(x_6[2].x);
    const int x_94 = A[x_92];
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_96 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_97 = (x_94 == x_96);
    x_98_phi = x_97;
  }
  if (x_98_phi) {
    const int x_103 = asint(x_6[3].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_106 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_109 = asint(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const int x_112 = asint(x_6[3].x);
    x_GLF_color = float4(float(x_103), float(x_106), float(x_109), float(x_112));
  } else {
    x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
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
