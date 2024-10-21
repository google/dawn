#version 310 es

void f() {
  {
    int must_not_collide = 0;
    while(true) {
      break;
    }
  }
  int must_not_collide = 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
