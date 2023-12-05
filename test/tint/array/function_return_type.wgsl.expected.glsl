#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  float inner;
} s;

float[4] f1() {
  float tint_symbol_6[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  return tint_symbol_6;
}

float[3][4] f2() {
  float tint_symbol_1[4] = f1();
  float tint_symbol_2[4] = f1();
  float tint_symbol_3[4] = f1();
  float tint_symbol_7[3][4] = float[3][4](tint_symbol_1, tint_symbol_2, tint_symbol_3);
  return tint_symbol_7;
}

float[2][3][4] f3() {
  float tint_symbol_4[3][4] = f2();
  float tint_symbol_5[3][4] = f2();
  float tint_symbol_8[2][3][4] = float[2][3][4](tint_symbol_4, tint_symbol_5);
  return tint_symbol_8;
}

void tint_symbol() {
  float a1[4] = f1();
  float a2[3][4] = f2();
  float a3[2][3][4] = f3();
  s.inner = ((a1[0] + a2[0][0]) + a3[0][0][0]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
