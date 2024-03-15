#version 310 es
precision highp float;
precision highp int;

bool tint_discarded = false;
layout(binding = 0, std430) buffer non_uniform_global_block_ssbo {
  int inner;
} non_uniform_global;

layout(binding = 1, std430) buffer tint_symbol_block_ssbo {
  float inner;
} tint_symbol;

void foo() {
  if ((non_uniform_global.inner < 0)) {
    tint_discarded = true;
  }
}

void bar() {
  float tint_symbol_2 = dFdx(1.0f);
  if (!(tint_discarded)) {
    tint_symbol.inner = tint_symbol_2;
  }
}

void tint_symbol_1() {
  foo();
  bar();
}

void main() {
  tint_symbol_1();
  if (tint_discarded) {
    discard;
  }
  return;
}
