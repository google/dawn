cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int r = 0;
  i = 0;
  r = 0;
  while (true) {
    const int x_35 = r;
    const int x_37 = asint(x_7[0].x);
    if ((x_35 < (x_37 * 4))) {
    } else {
      break;
    }
    const int x_41 = r;
    const int x_43 = asint(x_7[0].x);
    i = (i + int4(1, 2, 3, 4)[(x_41 / x_43)]);
    {
      r = (r + 2);
    }
  }
  if ((i == 10)) {
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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
