#version 310 es

layout(binding = 0, std140)
uniform x_block_1_ubo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  switch(v.inner) {
    case 0:
    {
      {
        while(true) {
          return;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}
