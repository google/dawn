struct S {
  int x;
  int y;
};

cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_struct_S_i1_i11_(inout S arg) {
  arg.y = 1;
  return;
}

void main_1() {
  float a = 0.0f;
  S b[2] = (S[2])0;
  S param = (S)0;
  a = 5.0f;
  while (true) {
    const int x_43 = asint(x_10[0].x);
    b[x_43].x = 1;
    const int x_46 = b[1].x;
    if ((x_46 == 1)) {
      const int x_51 = asint(x_10[0].x);
      if ((x_51 == 1)) {
        break;
      }
      const S x_56 = b[1];
      param = x_56;
      func_struct_S_i1_i11_(param);
      b[1] = param;
      const int x_61 = b[1].y;
      a = float(x_61);
    }
    a = 0.0f;
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  if ((a == 5.0f)) {
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
