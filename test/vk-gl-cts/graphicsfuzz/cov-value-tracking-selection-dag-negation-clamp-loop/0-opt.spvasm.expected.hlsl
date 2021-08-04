cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[16];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int ref[15] = (int[15])0;
  int i = 0;
  int data[15] = (int[15])0;
  int i_1 = 0;
  int i_2 = 0;
  int i_3 = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_46 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_48 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  ref[x_46] = x_48;
  const int x_51 = asint(x_6[1].x);
  const int x_53 = asint(x_6[1].x);
  ref[x_51] = x_53;
  const int x_56 = asint(x_6[2].x);
  const int x_58 = asint(x_6[2].x);
  ref[x_56] = x_58;
  const int x_61 = asint(x_6[3].x);
  const int x_63 = asint(x_6[3].x);
  ref[x_61] = x_63;
  const int x_66 = asint(x_6[4].x);
  const int x_68 = asint(x_6[4].x);
  ref[x_66] = x_68;
  const int x_71 = asint(x_6[5].x);
  const int x_73 = asint(x_6[1].x);
  ref[x_71] = -(x_73);
  const int x_77 = asint(x_6[8].x);
  const int x_79 = asint(x_6[1].x);
  ref[x_77] = -(x_79);
  const int x_83 = asint(x_6[9].x);
  const int x_85 = asint(x_6[1].x);
  ref[x_83] = -(x_85);
  const int x_89 = asint(x_6[10].x);
  const int x_91 = asint(x_6[1].x);
  ref[x_89] = -(x_91);
  const int x_95 = asint(x_6[11].x);
  const int x_97 = asint(x_6[1].x);
  ref[x_95] = -(x_97);
  const int x_101 = asint(x_6[6].x);
  const int x_103 = asint(x_6[2].x);
  ref[x_101] = -(x_103);
  const int x_107 = asint(x_6[12].x);
  const int x_109 = asint(x_6[2].x);
  ref[x_107] = -(x_109);
  const int x_113 = asint(x_6[13].x);
  const int x_115 = asint(x_6[2].x);
  ref[x_113] = -(x_115);
  const int x_119 = asint(x_6[14].x);
  const int x_121 = asint(x_6[2].x);
  ref[x_119] = -(x_121);
  const int x_125 = asint(x_6[15].x);
  const int x_127 = asint(x_6[2].x);
  ref[x_125] = -(x_127);
  i = 0;
  while (true) {
    const int x_134 = i;
    const int x_136 = asint(x_6[5].x);
    if ((x_134 < x_136)) {
    } else {
      break;
    }
    const int x_139 = i;
    const int x_140 = i;
    const int x_142 = i;
    const int x_145 = asint(x_6[1].x);
    data[x_139] = ~(clamp(~(x_140), ~(x_142), x_145));
    {
      i = (i + 1);
    }
  }
  const int x_152 = asint(x_6[5].x);
  i_1 = x_152;
  while (true) {
    const int x_157 = i_1;
    const int x_159 = asint(x_6[6].x);
    if ((x_157 < x_159)) {
    } else {
      break;
    }
    data[i_1] = ~(clamp(~(i_1), 0, 1));
    {
      i_1 = (i_1 + 1);
    }
  }
  const int x_171 = asint(x_6[6].x);
  i_2 = x_171;
  while (true) {
    const int x_176 = i_2;
    const int x_178 = asint(x_6[7].x);
    if ((x_176 < x_178)) {
    } else {
      break;
    }
    data[i_2] = ~(clamp(i_2, 0, 1));
    {
      i_2 = (i_2 + 1);
    }
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_189 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  i_3 = x_189;
  while (true) {
    const int x_194 = i_3;
    const int x_196 = asint(x_6[7].x);
    if ((x_194 < x_196)) {
    } else {
      break;
    }
    const int x_201 = data[i_3];
    const int x_204 = ref[i_3];
    if ((x_201 != x_204)) {
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const int x_209 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      const float x_210 = float(x_209);
      x_GLF_color = float4(x_210, x_210, x_210, x_210);
      return;
    }
    {
      i_3 = (i_3 + 1);
    }
  }
  const int x_215 = asint(x_6[1].x);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_218 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const int x_221 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const int x_224 = asint(x_6[1].x);
  x_GLF_color = float4(float(x_215), float(x_218), float(x_221), float(x_224));
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
