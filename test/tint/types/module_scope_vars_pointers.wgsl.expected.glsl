#version 310 es

float p = 0.0f;
shared float w;
void tint_symbol(uint local_invocation_index) {
  {
    w = 0.0f;
  }
  barrier();
  float x = (p + w);
  p = x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
