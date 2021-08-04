cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_i1_(inout int x) {
  const int x_41 = x;
  const int x_43 = asint(x_7[0].x);
  if ((x_41 < x_43)) {
    discard;
  }
  const int x_47 = x;
  if ((x_47 > 8)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

void main_1() {
  int i = 0;
  int param = 0;
  int x_31_phi = 0;
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  i = 0;
  x_31_phi = 0;
  while (true) {
    const int x_31 = x_31_phi;
    const int x_35 = asint(x_7[0].x);
    if ((x_31 < (10 + x_35))) {
    } else {
      break;
    }
    {
      param = x_31;
      func_i1_(param);
      const int x_32 = (x_31 + 1);
      i = x_32;
      x_31_phi = x_32;
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
