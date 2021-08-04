struct S {
  int data;
};

cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_struct_S_i11_i1_(inout S s, inout int x) {
  const int x_103 = asint(x_9[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_105 = asint(x_9[scalar_offset / 4][scalar_offset % 4]);
  if ((x_103 == x_105)) {
    return;
  }
  const int x_109 = x;
  s.data = x_109;
  return;
}

void main_1() {
  int i = 0;
  S arr[10] = (S[10])0;
  int index = 0;
  S param = (S)0;
  int param_1 = 0;
  S param_2 = (S)0;
  int param_3 = 0;
  i = 0;
  {
    for(; (i < 10); i = (i + 1)) {
      arr[i].data = 0;
    }
  }
  const int x_51 = asint(x_9[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_53 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_51 == x_53)) {
    const int x_58 = index;
    const S x_60 = arr[x_58];
    param = x_60;
    param_1 = index;
    func_struct_S_i11_i1_(param, param_1);
    arr[x_58] = param;
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_66 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const S x_68 = arr[x_66];
    param_2 = x_68;
    const int x_70 = asint(x_9[1].x);
    param_3 = x_70;
    func_struct_S_i11_i1_(param_2, param_3);
    arr[x_66] = param_2;
  }
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_75 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const int x_77 = arr[x_75].data;
  const int x_79 = asint(x_9[1].x);
  if ((x_77 == x_79)) {
    const int x_85 = asint(x_9[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_88 = asint(x_9[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_91 = asint(x_9[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_94 = asint(x_9[1].x);
    x_GLF_color = float4(float(x_85), float(x_88), float(x_91), float(x_94));
  } else {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_98 = asint(x_9[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_99 = float(x_98);
    x_GLF_color = float4(x_99, x_99, x_99, x_99);
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
