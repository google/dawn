static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_i1_(inout int x) {
  int a = 0;
  int data[9] = (int[9])0;
  int temp[2] = (int[2])0;
  int i = 0;
  bool x_95 = false;
  bool x_96_phi = false;
  a = 0;
  data[0] = 5;
  while (true) {
    const int x_56 = a;
    const int x_57 = x;
    if ((x_56 <= x_57)) {
    } else {
      break;
    }
    if ((a <= 10)) {
      const int x_64 = a;
      const int x_69 = data[min(a, 0)];
      temp[min(x_64, 1)] = x_69;
      a = (a + 1);
    }
  }
  i = 0;
  {
    for(; (i < 2); i = (i + 1)) {
      const int x_80 = i;
      const int x_82 = temp[0];
      data[x_80] = (x_82 + i);
    }
  }
  const int x_89 = data[0];
  const bool x_90 = (x_89 == 5);
  x_96_phi = x_90;
  if (x_90) {
    const int x_94 = data[1];
    x_95 = (x_94 == 6);
    x_96_phi = x_95;
  }
  if (x_96_phi) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

void main_1() {
  int i_1 = 0;
  int param = 0;
  i_1 = 1;
  {
    for(; (i_1 < 6); i_1 = (i_1 + 1)) {
      param = i_1;
      func_i1_(param);
    }
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
