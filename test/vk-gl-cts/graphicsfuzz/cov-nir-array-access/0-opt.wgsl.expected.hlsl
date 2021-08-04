cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[19];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int A[17] = (int[17])0;
  int ref[17] = (int[17])0;
  int a = 0;
  int i = 0;
  bool ok = false;
  int i_1 = 0;
  const int x_52 = asint(x_6[2].x);
  const int x_54 = asint(x_6[2].x);
  const int x_56 = asint(x_6[2].x);
  const int x_58 = asint(x_6[2].x);
  const int x_60 = asint(x_6[2].x);
  const int x_62 = asint(x_6[2].x);
  const int x_64 = asint(x_6[2].x);
  const int x_66 = asint(x_6[2].x);
  const int x_68 = asint(x_6[2].x);
  const int x_70 = asint(x_6[2].x);
  const int x_72 = asint(x_6[2].x);
  const int x_74 = asint(x_6[2].x);
  const int x_76 = asint(x_6[2].x);
  const int x_78 = asint(x_6[2].x);
  const int x_80 = asint(x_6[2].x);
  const int x_82 = asint(x_6[2].x);
  const int x_84 = asint(x_6[2].x);
  const int tint_symbol_2[17] = {x_52, x_54, x_56, x_58, x_60, x_62, x_64, x_66, x_68, x_70, x_72, x_74, x_76, x_78, x_80, x_82, x_84};
  A = tint_symbol_2;
  const int x_87 = asint(x_6[3].x);
  const int x_89 = asint(x_6[4].x);
  const int x_91 = asint(x_6[5].x);
  const int x_93 = asint(x_6[6].x);
  const int x_95 = asint(x_6[7].x);
  const int x_97 = asint(x_6[8].x);
  const int x_99 = asint(x_6[9].x);
  const int x_101 = asint(x_6[10].x);
  const int x_103 = asint(x_6[11].x);
  const int x_105 = asint(x_6[12].x);
  const int x_107 = asint(x_6[13].x);
  const int x_109 = asint(x_6[14].x);
  const int x_111 = asint(x_6[15].x);
  const int x_113 = asint(x_6[16].x);
  const int x_115 = asint(x_6[17].x);
  const int x_117 = asint(x_6[18].x);
  const int x_119 = asint(x_6[1].x);
  const int tint_symbol_3[17] = {x_87, x_89, x_91, x_93, x_95, x_97, x_99, x_101, x_103, x_105, x_107, x_109, x_111, x_113, x_115, x_117, x_119};
  ref = tint_symbol_3;
  const int x_122 = asint(x_6[2].x);
  a = x_122;
  const int x_124 = asint(x_6[2].x);
  i = x_124;
  while (true) {
    const int x_129 = i;
    const int x_131 = asint(x_6[1].x);
    if ((x_129 < x_131)) {
    } else {
      break;
    }
    const int x_134 = i;
    const int x_135 = a;
    a = (x_135 - 1);
    A[x_134] = x_135;
    const int x_138 = i;
    const int x_140 = asint(x_6[2].x);
    const int x_142 = asint(x_6[18].x);
    const int x_144 = i;
    const int x_146 = asint(x_6[3].x);
    A[clamp(x_138, x_140, x_142)] = (x_144 + x_146);
    {
      i = (i + 1);
    }
  }
  ok = true;
  const int x_152 = asint(x_6[2].x);
  i_1 = x_152;
  while (true) {
    const int x_157 = i_1;
    const int x_159 = asint(x_6[1].x);
    if ((x_157 < x_159)) {
    } else {
      break;
    }
    const int x_164 = A[i_1];
    const int x_167 = ref[i_1];
    if ((x_164 != x_167)) {
      ok = false;
    }
    {
      i_1 = (i_1 + 1);
    }
  }
  const int x_174 = asint(x_6[2].x);
  const float x_175 = float(x_174);
  x_GLF_color = float4(x_175, x_175, x_175, x_175);
  if (ok) {
    const int x_181 = asint(x_6[3].x);
    const int x_184 = asint(x_6[2].x);
    const int x_187 = asint(x_6[2].x);
    const int x_190 = asint(x_6[3].x);
    x_GLF_color = float4(float(x_181), float(x_184), float(x_187), float(x_190));
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
