#version 310 es

layout(binding = 0, std140)
uniform b_block_1_ubo {
  int inner;
} v;
bool func_3() {
  {
    int i = 0;
    while(true) {
      if ((i < v.inner)) {
      } else {
        break;
      }
      {
        int j = -1;
        while(true) {
          if ((j == 1)) {
          } else {
            break;
          }
          return false;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  return false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func_3();
}
