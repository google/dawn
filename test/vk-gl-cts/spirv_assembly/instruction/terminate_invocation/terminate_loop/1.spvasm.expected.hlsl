static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int x_3 = 0;
static int x_4 = 0;

void main_1() {
  int x_33_phi = 0;
  const float4 x_18 = x_2;
  const int x_28 = x_3;
  x_33_phi = 0;
  if (((((int(x_18.x) & 1) + (int(x_18.y) & 1)) + x_28) == int(x_18.z))) {
    while (true) {
      int x_34 = 0;
      const int x_33 = x_33_phi;
      if ((asuint(x_33) < asuint(10))) {
      } else {
        break;
      }
      {
        x_34 = (x_33 + 1);
        x_33_phi = x_34;
      }
    }
  }
  x_4 = 1;
  return;
}

struct main_out {
  int x_4_1;
};
struct tint_symbol_1 {
  int x_3_param : TEXCOORD0;
  float4 x_2_param : SV_Position;
};
struct tint_symbol_2 {
  int x_4_1 : SV_Target0;
};

main_out main_inner(float4 x_2_param, int x_3_param) {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  const main_out tint_symbol_3 = {x_4};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.x_2_param, tint_symbol.x_3_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_4_1 = inner_result.x_4_1;
  return wrapper_result;
}
