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
  const float x_166 = asfloat(x_8[0].x);
  const float x_168 = asfloat(x_8[0].y);
  if ((x_166 > x_168)) {
    return;
  }
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_173 = asint(x_10[scalar_offset / 4][scalar_offset % 4]);
  s.data = x_173;
  return;
}

void main_1() {
  int i = 0;
  S arr[3] = (S[3])0;
  int i_1 = 0;
  S param = (S)0;
  int j = 0;
  bool x_132 = false;
  bool x_142 = false;
  bool x_133_phi = false;
  bool x_143_phi = false;
  const int x_46 = asint(x_10[2].x);
  i = x_46;
  while (true) {
    const int x_51 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_53 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
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
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_69 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
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
    const int x_83 = asint(x_10[3].x);
    if ((x_81 == x_83)) {
      const int x_88 = i_1;
      const S x_90 = arr[x_88];
      param = x_90;
      func_struct_S_i11_(param);
      arr[x_88] = param;
    } else {
      const int x_95 = asint(x_10[2].x);
      j = x_95;
      while (true) {
        const int x_100 = j;
        const uint scalar_offset_3 = ((16u * uint(0))) / 4;
        const int x_102 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
        if ((x_100 < x_102)) {
        } else {
          break;
        }
        const int x_107 = arr[j].data;
        const int x_109 = asint(x_10[4].x);
        if ((x_107 > x_109)) {
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
  const int x_118 = asint(x_10[2].x);
  const int x_120 = arr[x_118].data;
  const int x_122 = asint(x_10[2].x);
  const bool x_123 = (x_120 == x_122);
  x_133_phi = x_123;
  if (x_123) {
    const int x_127 = asint(x_10[3].x);
    const int x_129 = arr[x_127].data;
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_131 = asint(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_132 = (x_129 == x_131);
    x_133_phi = x_132;
  }
  const bool x_133 = x_133_phi;
  x_143_phi = x_133;
  if (x_133) {
    const int x_137 = asint(x_10[1].x);
    const int x_139 = arr[x_137].data;
    const int x_141 = asint(x_10[1].x);
    x_142 = (x_139 == x_141);
    x_143_phi = x_142;
  }
  if (x_143_phi) {
    const int x_148 = asint(x_10[3].x);
    const int x_151 = asint(x_10[2].x);
    const int x_154 = asint(x_10[2].x);
    const int x_157 = asint(x_10[3].x);
    x_GLF_color = float4(float(x_148), float(x_151), float(x_154), float(x_157));
  } else {
    const int x_161 = asint(x_10[2].x);
    const float x_162 = float(x_161);
    x_GLF_color = float4(x_162, x_162, x_162, x_162);
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
