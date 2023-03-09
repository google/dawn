#version 310 es
precision highp float;

bool tint_discarded = false;
layout(binding = 0, std430) buffer non_uniform_global_block_ssbo {
  int inner;
} non_uniform_global;

void tint_symbol() {
  if ((non_uniform_global.inner < 0)) {
    tint_discarded = true;
  }
  {
    {
      return;
    }
  }
}

void main() {
  tint_symbol();
  if (tint_discarded) {
    discard;
  }
  return;
}
