#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  int tint_symbol_1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  switch(v.tint_symbol_1) {
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
