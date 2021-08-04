cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_() {
  int ret = 0;
  int i = 0;
  ret = 0;
  i = 3;
  while (true) {
    if ((i > (i & 1))) {
    } else {
      break;
    }
    ret = (ret + 1);
    {
      const int x_47 = asint(x_8[0].x);
      i = (i - x_47);
    }
  }
  return ret;
}

void main_1() {
  const int x_29 = func_();
  if ((x_29 == 2)) {
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
