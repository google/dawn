static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int x_3 = 0;
static int x_4 = 0;
RWByteAddressBuffer x_5 : register(u0, space0);

void main_1() {
  x_4 = 1;
  const float4 x_23 = x_2;
  const int x_27 = int(x_23.x);
  const int x_28 = int(x_23.y);
  const int x_33 = x_3;
  x_5.Store((4u * uint((x_27 + (x_28 * 8)))), asuint(x_27));
  if (((((x_27 & 1) + (x_28 & 1)) + x_33) == int(x_23.z))) {
  }
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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 x_2_param = tint_symbol.x_2_param;
  const int x_3_param = tint_symbol.x_3_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  const main_out tint_symbol_3 = {x_4};
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.x_4_1};
  return tint_symbol_5;
}
