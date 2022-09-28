#version 310 es

layout(binding = 0, std140) uniform x_block_ubo {
  int inner;
} x;

void tint_symbol() {
  switch(x.inner) {
    case 0: {
      while (true) {
        return;
      }
      break;
    }
    default: {
      break;
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
