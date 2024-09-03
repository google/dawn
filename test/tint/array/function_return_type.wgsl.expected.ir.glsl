#version 310 es

float s;
float[4] f1() {
  return float[4](0.0f, 0.0f, 0.0f, 0.0f);
}
float[3][4] f2() {
  float v[4] = f1();
  float v_1[4] = f1();
  return float[3][4](v, v_1, f1());
}
float[2][3][4] f3() {
  float v_2[3][4] = f2();
  return float[2][3][4](v_2, f2());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a1[4] = f1();
  float a2[3][4] = f2();
  float a3[2][3][4] = f3();
  s = ((a1[0] + a2[0][0]) + a3[0][0][0]);
}
