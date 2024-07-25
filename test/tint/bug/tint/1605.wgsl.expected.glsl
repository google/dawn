#version 310 es

layout(binding = 0, std140) uniform b_block_ubo {
  int inner;
} b;

bool func_3() {
  {
    for(int i = 0; (i < b.inner); i = (i + 1)) {
      {
        for(int j = -1; (j == 1); j = (j + 1)) {
          return false;
        }
      }
    }
  }
  return false;
}

void tint_symbol() {
  func_3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
