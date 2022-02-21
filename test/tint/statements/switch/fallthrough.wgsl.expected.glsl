#version 310 es

void f() {
  int i = 0;
  switch(i) {
    case 0: {
      /* fallthrough */
    }
    default: {
      break;
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
