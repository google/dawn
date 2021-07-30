struct Communicators {
  float alice;
  float4 bob;
};

static Communicators x_1 = (Communicators)0;
static Communicators x_3 = (Communicators)0;
static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  return;
}

struct main_out {
  float4 x_2_1;
  float x_3_1;
  float4 x_3_2;
};
struct tint_symbol_1 {
  float x_1_param : TEXCOORD9;
  float4 x_1_param_1 : TEXCOORD11;
};
struct tint_symbol_2 {
  float x_3_1 : TEXCOORD9;
  float4 x_3_2 : TEXCOORD11;
  float4 x_2_1 : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float x_1_param = tint_symbol.x_1_param;
  const float4 x_1_param_1 = tint_symbol.x_1_param_1;
  x_1.alice = x_1_param;
  x_1.bob = x_1_param_1;
  main_1();
  const main_out tint_symbol_3 = {x_2, x_3.alice, x_3.bob};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_3_1, tint_symbol_3.x_3_2, tint_symbol_3.x_2_1};
  return tint_symbol_4;
}
