#version 310 es
precision mediump float;

float[4] f1() {
  float tint_symbol_1[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  return tint_symbol_1;
}

float[3][4] f2() {
  float tint_symbol_2[3][4] = float[3][4](f1(), f1(), f1());
  return tint_symbol_2;
}

float[2][3][4] f3() {
  float tint_symbol_3[2][3][4] = float[2][3][4](f2(), f2());
  return tint_symbol_3;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  float a1[4] = f1();
  float a2[3][4] = f2();
  float a3[2][3][4] = f3();
  return;
}
void main() {
  tint_symbol();
}


