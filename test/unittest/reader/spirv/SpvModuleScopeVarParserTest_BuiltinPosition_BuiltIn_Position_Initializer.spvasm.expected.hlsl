static float4 gl_Position = float4(1.0f, 2.0f, 3.0f, 4.0f);

void main_1() {
  return;
}

struct main_out {
  float4 gl_Position;
};
struct tint_symbol {
  float4 gl_Position : SV_Position;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {gl_Position};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.gl_Position};
  return tint_symbol_2;
}
