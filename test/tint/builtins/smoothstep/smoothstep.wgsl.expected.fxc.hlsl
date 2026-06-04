
[numthreads(1, 1, 1)]
void main() {
  float low = 1.0f;
  float high = 0.0f;
  float x_val = 0.5f;
  float v = clamp(((x_val - low) / (high - low)), 0.0f, 1.0f);
  float res = (v * (v * (3.0f - (2.0f * v))));
}

