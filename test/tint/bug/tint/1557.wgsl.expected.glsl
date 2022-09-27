#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  int inner;
} u;

int f() {
  return 0;
}

void g() {
  int j = 0;
  while (true) {
    if ((j >= 1)) {
      break;
    }
    j = (j + 1);
    int k = f();
  }
}

void tint_symbol() {
  switch(u.inner) {
    case 0: {
      switch(u.inner) {
        case 0: {
          break;
        }
        default: {
          g();
          break;
        }
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
