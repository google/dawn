cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int b = 0;
  int i = 0;
  a = 0;
  b = 0;
  i = 0;
  while (true) {
    const int x_36 = i;
    const int x_38 = asint(x_8[0].x);
    if ((x_36 < x_38)) {
    } else {
      break;
    }
    if ((a > 5)) {
      break;
    }
    const int x_46 = asint(x_10[0].x);
    a = (a + (x_46 / -4));
    b = (b + 1);
    {
      i = (i + 1);
    }
  }
  if ((b == 3)) {
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
