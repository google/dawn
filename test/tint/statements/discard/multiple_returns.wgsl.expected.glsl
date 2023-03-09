#version 310 es
precision highp float;

bool tint_discarded = false;
layout(binding = 0, std430) buffer non_uniform_global_block_ssbo {
  int inner;
} non_uniform_global;

layout(binding = 1, std430) buffer tint_symbol_block_ssbo {
  float inner;
} tint_symbol;

void tint_symbol_1() {
  if ((non_uniform_global.inner < 0)) {
    tint_discarded = true;
  }
  if (!(tint_discarded)) {
    tint_symbol.inner = dFdx(1.0f);
  }
  if ((tint_symbol.inner < 0.0f)) {
    int i = 0;
    while (true) {
      if ((tint_symbol.inner > float(i))) {
        if (!(tint_discarded)) {
          tint_symbol.inner = float(i);
        }
        return;
      }
      {
        i = (i + 1);
        if ((i == 5)) { break; }
      }
    }
    return;
  }
  return;
}

void main() {
  tint_symbol_1();
  if (tint_discarded) {
    discard;
  }
  return;
}
