#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float low = 1.0f;
  float high = 0.0f;
  float x_val = 0.5f;
  float v = clamp(((x_val - low) / (high - low)), 0.0f, 1.0f);
  float res = (v * (v * (3.0f - (2.0f * v))));
}
