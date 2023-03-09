#version 310 es
precision highp float;

bool tint_discarded = false;
layout(location = 0) in float tint_symbol_1;
layout(location = 1) in vec2 coord_1;
layout(location = 0) out int value;
int tint_ftoi(float v) {
  return ((v < 2147483520.0f) ? ((v < -2147483648.0f) ? (-2147483647 - 1) : int(v)) : 2147483647);
}

layout(binding = 2, std430) buffer a_block_ssbo {
  int inner;
} a;

uniform highp sampler2D t_s;

int foo(float tint_symbol, vec2 coord) {
  if ((tint_symbol == 0.0f)) {
    tint_discarded = true;
  }
  int result = tint_ftoi(texture(t_s, coord).x);
  {
    int i = 0;
    while (true) {
      if (!((i < 10))) {
        break;
      }
      {
        result = (result + i);
      }
      {
        int tint_symbol_2 = 0;
        if (!(tint_discarded)) {
          tint_symbol_2 = atomicAdd(a.inner, 1);
        }
        i = tint_symbol_2;
      }
    }
  }
  return result;
}

void main() {
  int inner_result = foo(tint_symbol_1, coord_1);
  value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return;
}
