struct S {
  int data;
};

cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_struct_S_i11_(inout S s) {
  while (true) {
    const float x_174 = asfloat(x_8[0].x);
    const float x_176 = asfloat(x_8[0].y);
    if ((x_174 > x_176)) {
    } else {
      break;
    }
    return;
  }
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_180 = asint(x_10[scalar_offset / 4][scalar_offset % 4]);
  s.data = x_180;
  return;
}

void main_1() {
  int i = 0;
  S arr[3] = (S[3])0;
  int i_1 = 0;
  S param = (S)0;
  int j = 0;
  bool x_136 = false;
  bool x_146 = false;
  bool x_137_phi = false;
  bool x_147_phi = false;
  const int x_46 = asint(x_10[2].x);
  i = x_46;
  while (true) {
    const int x_51 = i;
    const int x_53 = asint(x_10[1].x);
    if ((x_51 < x_53)) {
    } else {
      break;
    }
    arr[i].data = i;
    {
      i = (i + 1);
    }
  }
  const int x_62 = asint(x_10[2].x);
  i_1 = x_62;
  while (true) {
    const int x_67 = i_1;
    const int x_69 = asint(x_10[1].x);
    if ((x_67 < x_69)) {
    } else {
      break;
    }
    const float x_73 = asfloat(x_8[0].x);
    const float x_75 = asfloat(x_8[0].y);
    if ((x_73 > x_75)) {
      break;
    }
    const int x_81 = arr[i_1].data;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_83 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_81 == x_83)) {
      const int x_88 = i_1;
      const int x_91 = asint(x_10[3].x);
      arr[clamp(x_88, 0, 3)].data = x_91;
      const S x_94 = arr[2];
      param = x_94;
      func_struct_S_i11_(param);
      arr[2] = param;
    } else {
      const int x_99 = asint(x_10[2].x);
      j = x_99;
      while (true) {
        const int x_104 = j;
        const int x_106 = asint(x_10[1].x);
        if ((x_104 < x_106)) {
        } else {
          break;
        }
        const int x_111 = arr[j].data;
        const int x_113 = asint(x_10[4].x);
        if ((x_111 > x_113)) {
          discard;
        }
        {
          j = (j + 1);
        }
      }
    }
    {
      i_1 = (i_1 + 1);
    }
  }
  const int x_122 = asint(x_10[2].x);
  const int x_124 = arr[x_122].data;
  const int x_126 = asint(x_10[2].x);
  const bool x_127 = (x_124 == x_126);
  x_137_phi = x_127;
  if (x_127) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_131 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_133 = arr[x_131].data;
    const int x_135 = asint(x_10[3].x);
    x_136 = (x_133 == x_135);
    x_137_phi = x_136;
  }
  const bool x_137 = x_137_phi;
  x_147_phi = x_137;
  if (x_137) {
    const int x_141 = asint(x_10[3].x);
    const int x_143 = arr[x_141].data;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_145 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_146 = (x_143 == x_145);
    x_147_phi = x_146;
  }
  if (x_147_phi) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_152 = asint(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const int x_155 = asint(x_10[2].x);
    const int x_158 = asint(x_10[2].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_161 = asint(x_10[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(float(x_152), float(x_155), float(x_158), float(x_161));
  } else {
    const int x_165 = asint(x_10[2].x);
    const float x_166 = float(x_165);
    x_GLF_color = float4(x_166, x_166, x_166, x_166);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
