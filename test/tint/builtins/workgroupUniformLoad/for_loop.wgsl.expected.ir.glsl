#version 310 es

shared int a;
shared int b;
void foo() {
  {
    int i = 0;
    while(true) {
      int v = i;
      barrier();
      int v_1 = a;
      barrier();
      if ((v < v_1)) {
      } else {
        break;
      }
      {
        barrier();
        int v_2 = b;
        barrier();
        i = (i + v_2);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
