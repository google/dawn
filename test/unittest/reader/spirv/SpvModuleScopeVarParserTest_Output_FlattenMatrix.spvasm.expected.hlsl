static float2x4 x_1 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  return;
}

struct main_out {
  float4 x_1_1;
  float4 x_1_2;
  float4 x_2_1;
};
struct tint_symbol {
  float4 x_1_1 : TEXCOORD9;
  float4 x_1_2 : TEXCOORD10;
  float4 x_2_1 : SV_Position;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_1[0], x_1[1], x_2};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_1_1, tint_symbol_1.x_1_2, tint_symbol_1.x_2_1};
  return tint_symbol_2;
}
