struct S {
  int data;
};

cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_struct_S_i11_i1_(inout S s, inout int x) {
  const int x_17 = s.data;
  if ((x_17 == 1)) {
    const int x_18 = x;
    const int x_19 = s.data;
    return (x_18 + x_19);
  } else {
    const int x_21 = x;
    return x_21;
  }
  return 0;
}

void main_1() {
  int a = 0;
  S arr[1] = (S[1])0;
  int i = 0;
  S param = (S)0;
  int param_1 = 0;
  S param_2 = (S)0;
  int param_3 = 0;
  a = 0;
  const int x_22 = asint(x_11[0].x);
  arr[0].data = x_22;
  i = 0;
  while (true) {
    const int x_23 = i;
    const int x_24 = asint(x_11[0].x);
    if ((x_23 < (5 + x_24))) {
    } else {
      break;
    }
    if (((i % 2) != 0)) {
      const S x_74 = arr[0];
      param = x_74;
      param_1 = i;
      const int x_29 = func_struct_S_i11_i1_(param, param_1);
      arr[0] = param;
      a = x_29;
    } else {
      const S x_78 = arr[0];
      param_2 = x_78;
      param_3 = 1;
      const int x_30 = func_struct_S_i11_i1_(param_2, param_3);
      arr[0] = param_2;
      a = x_30;
    }
    {
      i = (i + 1);
    }
  }
  if ((a == 6)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
