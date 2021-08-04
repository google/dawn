struct S {
  int arr[2];
};

cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_struct_S_i1_2_1_i1_(inout S s, inout int x) {
  const int x_16 = x;
  s.arr[1] = (x_16 + 1);
  const int x_18 = asint(x_9[0].x);
  const int x_19 = s.arr[x_18];
  const int x_20 = x;
  if ((x_19 == x_20)) {
    return -1;
  }
  const int x_21 = x;
  return x_21;
}

void main_1() {
  int a = 0;
  int i = 0;
  int j = 0;
  S s_1 = (S)0;
  S param = (S)0;
  int param_1 = 0;
  a = 0;
  i = 0;
  while (true) {
    const int x_22 = i;
    const int x_23 = asint(x_9[0].x);
    if ((x_22 < (2 + x_23))) {
    } else {
      break;
    }
    j = 0;
    while (true) {
      const int x_25 = j;
      const int x_26 = asint(x_9[0].x);
      if ((x_25 < (3 + x_26))) {
      } else {
        break;
      }
      const int x_28 = i;
      const int x_29 = j;
      param = s_1;
      param_1 = (x_28 + x_29);
      const int x_31 = func_struct_S_i1_2_1_i1_(param, param_1);
      a = (a + x_31);
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
  if ((a == 30)) {
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
