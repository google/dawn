#version 310 es

float f1(float a[4]) {
  return a[3];
}

float f2(float a[3][4]) {
  return a[2][3];
}

float f3(float a[2][3][4]) {
  return a[1][2][3];
}

void tint_symbol() {
  float a1[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float a2[3][4] = float[3][4](float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f));
  float a3[2][3][4] = float[2][3][4](float[3][4](float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f)), float[3][4](float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f)));
  float v1 = f1(a1);
  float v2 = f2(a2);
  float v3 = f3(a3);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
