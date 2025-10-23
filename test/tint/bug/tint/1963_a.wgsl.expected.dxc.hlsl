
void X(float2 a, float2 b) {
}

float2 Y() {
  return (0.0f).xx;
}

[numthreads(1, 1, 1)]
void f() {
  float2 v = (0.0f).xx;
  X((0.0f).xx, v);
  X((0.0f).xx, Y());
}

