#version 310 es

layout(binding = 1, std430) buffer buf_block_ssbo {
  uint inner[1];
} buf;

int g() {
  return 0;
}

int f() {
  while (true) {
    g();
    break;
  }
  int o = g();
  return 0;
}

void tint_symbol() {
  while (true) {
    if ((buf.inner[0] == 0u)) {
      break;
    }
    int s = f();
    buf.inner[0] = 0u;
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
