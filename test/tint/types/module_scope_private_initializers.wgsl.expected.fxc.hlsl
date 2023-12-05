static float a = 1.0f;
static float b = 0.0f;

[numthreads(1, 1, 1)]
void main() {
  const float x = (a + b);
  return;
}
