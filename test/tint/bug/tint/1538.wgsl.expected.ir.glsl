#version 310 es

uint buf[1];
int g() {
  return 0;
}
int f() {
  {
    while(true) {
      g();
      break;
    }
  }
  int o = g();
  return 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    while(true) {
      if ((buf[0] == 0u)) {
        break;
      }
      int s = f();
      buf[0] = 0u;
      {
      }
      continue;
    }
  }
}
