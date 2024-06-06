#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  int inner[4];
} tint_symbol;

void foo_tint_symbol_inner() {
  bool tint_continue = false;
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
      tint_continue = false;
      switch(tint_symbol.inner[i]) {
        case 1: {
          tint_continue = true;
          break;
        }
        default: {
          tint_symbol.inner[i] = 2;
          break;
        }
      }
      if (tint_continue) {
        continue;
      }
    }
  }
}

void tint_symbol_1() {
  foo_tint_symbol_inner();
}

void main() {
  tint_symbol_1();
  return;
}
