static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
