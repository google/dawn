#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float b(int i) {
  return 2.29999995231628417969f;
}

int c(uint u) {
  return 1;
}

void a() {
  int tint_symbol = c(2u);
  float a_1 = b(tint_symbol);
}

