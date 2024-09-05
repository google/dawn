#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  int tint_symbol_2[4];
} v;
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      switch(v.tint_symbol_2[i]) {
        case 1:
        {
          {
            i = (i + 1);
          }
          continue;
        }
        default:
        {
          v.tint_symbol_2[i] = 2;
          break;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
void main() {
  foo();
}
