#version 310 es

uniform int b;
bool func_3() {
  {
    int i = 0;
    while(true) {
      if ((i < b)) {
      } else {
        break;
      }
      {
        int j = -1;
        while(true) {
          if ((j == 1)) {
          } else {
            break;
          }
          return false;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  return false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func_3();
}
