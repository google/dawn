#version 310 es

void compute_main() {
  float a = 1.230000019f;
  float b = max(a, 1.17549435e-38f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
