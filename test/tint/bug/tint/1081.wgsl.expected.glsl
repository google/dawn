#version 310 es
precision highp float;
precision highp int;

bool tint_discarded = false;
layout(location = 1) flat in ivec3 x_1;
layout(location = 2) out int value;
int f(int x) {
  if ((x == 10)) {
    tint_discarded = true;
  }
  return x;
}

int tint_symbol(ivec3 x) {
  int y = x.x;
  while (true) {
    int r = f(y);
    if ((r == 0)) {
      break;
    }
  }
  return y;
}

void main() {
  int inner_result = tint_symbol(x_1);
  value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return;
}
