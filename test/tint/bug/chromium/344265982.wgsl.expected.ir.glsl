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
      int v_1 = i;
      bool tint_continue = false;
      switch(v.tint_symbol_2[v_1]) {
        case 1:
        {
          tint_continue = true;
          break;
        }
        default:
        {
          int v_2 = i;
          v.tint_symbol_2[v_2] = 2;
          break;
        }
      }
      if (tint_continue) {
        {
          i = (i + 1);
        }
        continue;
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
