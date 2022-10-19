#version 310 es

void f() {
  int i = 0;
  int result = 0;
  switch(i) {
    case 0: {
      result = 10;
      break;
    }
    case 1:
    default: {
      result = 22;
      break;
    }
    case 2: {
      result = 33;
      break;
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
