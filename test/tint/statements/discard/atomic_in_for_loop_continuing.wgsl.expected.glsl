#version 310 es
precision highp float;
precision highp int;

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

bool tint_discarded = false;
layout(location = 0) in float tint_symbol_2;
layout(location = 1) in vec2 coord_1;
layout(location = 0) out int value;
layout(binding = 2, std430) buffer a_block_ssbo {
  int inner;
} a;

uniform highp sampler2D t_s;

int foo(float tint_symbol, vec2 coord) {
  if ((tint_symbol == 0.0f)) {
    tint_discarded = true;
  }
  vec4 tint_symbol_1 = texture(t_s, coord);
  int result = tint_ftoi(tint_symbol_1.x);
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
        int tint_symbol_3 = 0;
        if (!(tint_discarded)) {
          tint_symbol_3 = atomicAdd(a.inner, 1);
        }
        i = tint_symbol_3;
      }
    }
  }
  return result;
}

void main() {
  int inner_result = foo(tint_symbol_2, coord_1);
  value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return;
}
