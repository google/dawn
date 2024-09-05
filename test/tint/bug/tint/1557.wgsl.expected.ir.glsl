#version 310 es

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  int tint_symbol_1;
} v;
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
  switch(v.tint_symbol_1) {
    case 0:
    {
      switch(v.tint_symbol_1) {
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
