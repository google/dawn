struct S {
  int a;
  int b;
  int c;
};

cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_43 = 0;
  bool x_44 = false;
  S arr[2] = (S[2])0;
  S param = (S)0;
  int param_1 = 0;
  while (true) {
    int x_50 = 0;
    x_50 = asint(x_10[0].x);
    arr[x_50].a = 2;
    const int x_53 = arr[1].a;
    if ((x_53 < 1)) {
      x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
      x_44 = true;
      break;
    } else {
      const S x_60 = arr[1];
      param = x_60;
      param_1 = (2 + asint(x_50));
      const int x_61 = param_1;
      S x_64_1 = param;
      x_64_1.a = x_61;
      param = x_64_1;
      if ((param.a == 2)) {
        S x_71_1 = param;
        x_71_1.a = 9;
        param = x_71_1;
      }
      const int x_72 = param_1;
      S x_76_1 = param;
      x_76_1.b = (x_72 + 1);
      param = x_76_1;
      const int x_77 = param_1;
      S x_81_1 = param;
      x_81_1.c = (x_77 + 2);
      param = x_81_1;
      if ((param.b == 2)) {
        S x_88_1 = param;
        x_88_1.b = 7;
        param = x_88_1;
      }
      x_43 = ((param.a + param.b) + param.c);
      if ((x_43 == 12)) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      } else {
        x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
      }
    }
    x_44 = true;
    break;
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

int func_struct_S_i1_i1_i11_i1_(inout S s, inout int x) {
  const int x_103 = x;
  s.a = x_103;
  const int x_105 = s.a;
  if ((x_105 == 2)) {
    s.a = 9;
  }
  const int x_109 = x;
  s.b = (x_109 + 1);
  const int x_112 = x;
  s.c = (x_112 + 2);
  const int x_115 = s.b;
  if ((x_115 == 2)) {
    s.b = 7;
  }
  const int x_119 = s.a;
  const int x_120 = s.b;
  const int x_122 = s.c;
  return ((x_119 + x_120) + x_122);
}
