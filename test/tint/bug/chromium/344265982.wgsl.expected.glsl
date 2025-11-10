#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_buffer_block_ssbo {
  int inner[4];
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
      switch(v.inner[v_1]) {
        case 1:
        {
          tint_continue = true;
          break;
        }
        default:
        {
          int v_2 = i;
          v.inner[v_2] = 2;
          break;
        }
      }
      if (tint_continue) {
        {
          uint v_3 = uint(i);
          i = int((v_3 + uint(1)));
        }
        continue;
      }
      {
        uint v_3 = uint(i);
        i = int((v_3 + uint(1)));
      }
      continue;
    }
  }
}
void main() {
  foo();
}
