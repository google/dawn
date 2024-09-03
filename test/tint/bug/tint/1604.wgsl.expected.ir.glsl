#version 310 es

uniform int x;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  switch(x) {
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
