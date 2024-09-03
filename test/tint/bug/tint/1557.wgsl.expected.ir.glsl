#version 310 es

uniform int u;
int f() {
  return 0;
}
void g() {
  int j = 0;
  {
    while(true) {
      if ((j >= 1)) {
        break;
      }
      j = (j + 1);
      int k = f();
      {
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  switch(u) {
    case 0:
    {
      switch(u) {
        case 0:
        {
          break;
        }
        default:
        {
          g();
          break;
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
