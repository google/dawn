cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[12];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int f_i1_(inout int a) {
  int i = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_16 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
  i = x_16;
  while (true) {
    const int x_17 = i;
    const int x_18 = asint(x_8[6].x);
    if ((x_17 < x_18)) {
    } else {
      break;
    }
    const int x_19 = i;
    const int x_20 = asint(x_8[2].x);
    if ((x_19 > x_20)) {
      const int x_21 = a;
      return x_21;
    }
    {
      i = (i + 1);
    }
  }
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_24 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  return x_24;
}

void main_1() {
  int ref[10] = (int[10])0;
  int i_1 = 0;
  int a_1[10] = (int[10])0;
  int param = 0;
  int param_1 = 0;
  int i_2 = 0;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_25 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int x_26 = asint(x_8[1].x);
  ref[x_25] = x_26;
  const int x_27 = asint(x_8[11].x);
  const int x_28 = asint(x_8[2].x);
  ref[x_27] = x_28;
  const int x_29 = asint(x_8[1].x);
  const int x_30 = asint(x_8[3].x);
  ref[x_29] = x_30;
  const int x_31 = asint(x_8[2].x);
  const int x_32 = asint(x_8[4].x);
  ref[x_31] = x_32;
  const int x_33 = asint(x_8[3].x);
  const int x_34 = asint(x_8[5].x);
  ref[x_33] = x_34;
  const int x_35 = asint(x_8[4].x);
  const int x_36 = asint(x_8[6].x);
  ref[x_35] = x_36;
  const int x_37 = asint(x_8[5].x);
  const int x_38 = asint(x_8[7].x);
  ref[x_37] = x_38;
  const int x_39 = asint(x_8[8].x);
  const int x_40 = asint(x_8[8].x);
  ref[x_39] = x_40;
  const int x_41 = asint(x_8[9].x);
  const int x_42 = asint(x_8[9].x);
  ref[x_41] = x_42;
  const int x_43 = asint(x_8[10].x);
  const int x_44 = asint(x_8[10].x);
  ref[x_43] = x_44;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_45 = asint(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  i_1 = x_45;
  while (true) {
    const int x_46 = i_1;
    const int x_47 = asint(x_8[6].x);
    if ((x_46 < x_47)) {
    } else {
      break;
    }
    a_1[i_1] = i_1;
    const int x_50 = i_1;
    const int x_51 = asint(x_8[6].x);
    const int x_52 = asint(x_8[1].x);
    if ((x_50 < (x_51 / x_52))) {
      const int x_54 = i_1;
      const int x_55 = i_1;
      const int x_56 = asint(x_8[1].x);
      a_1[x_54] = (x_55 + x_56);
      const int x_58 = i_1;
      const int x_59 = asint(x_8[6].x);
      if ((x_58 < x_59)) {
        {
          i_1 = (i_1 + 1);
        }
        continue;
      }
      const int x_60 = i_1;
      const int x_61 = i_1;
      const int x_62 = asint(x_8[8].x);
      a_1[x_60] = (x_61 + x_62);
      const int x_65 = a_1[i_1];
      param = x_65;
      const int x_66 = f_i1_(param);
      const int x_67 = asint(x_8[8].x);
      if ((x_66 < x_67)) {
        const int x_182_save = i_1;
        const int x_69 = a_1[x_182_save];
        a_1[x_182_save] = (x_69 - 1);
      }
    } else {
      const int x_72 = a_1[i_1];
      param_1 = x_72;
      const int x_73 = f_i1_(param_1);
      const int x_74 = asint(x_8[8].x);
      if ((x_73 < x_74)) {
        const int x_75 = i_1;
        const int x_76 = asint(x_8[4].x);
        const int x_77 = a_1[x_75];
        a_1[x_75] = (x_77 + x_76);
      }
    }
    {
      i_1 = (i_1 + 1);
    }
  }
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_81 = asint(x_8[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  i_2 = x_81;
  while (true) {
    const int x_82 = i_2;
    const int x_83 = asint(x_8[6].x);
    if ((x_82 < x_83)) {
    } else {
      break;
    }
    const int x_85 = a_1[i_2];
    const int x_87 = ref[i_2];
    if ((x_85 != x_87)) {
      const uint scalar_offset_5 = ((16u * uint(0))) / 4;
      const int x_88 = asint(x_8[scalar_offset_5 / 4][scalar_offset_5 % 4]);
      const float x_205 = float(x_88);
      x_GLF_color = float4(x_205, x_205, x_205, x_205);
      return;
    }
    {
      i_2 = (i_2 + 1);
    }
  }
  const int x_91 = asint(x_8[11].x);
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const int x_92 = asint(x_8[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  const uint scalar_offset_7 = ((16u * uint(0))) / 4;
  const int x_93 = asint(x_8[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  const int x_94 = asint(x_8[11].x);
  x_GLF_color = float4(float(x_91), float(x_92), float(x_93), float(x_94));
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
