#version 310 es
precision mediump float;

layout(location = 1) flat in ivec3 x_1;
layout(location = 2) out int value;
bool tint_discard = false;
int f(int x) {
  if ((x == 10)) {
    tint_discard = true;
    return 0;
  }
  return x;
}

int tint_symbol(ivec3 x) {
  int y = x.x;
  while (true) {
    int r = f(y);
    if (tint_discard) {
      return 0;
    }
    if ((r == 0)) {
      break;
    }
  }
  return y;
}

void tint_discard_func() {
  discard;
}

void main() {
  int inner_result = tint_symbol(x_1);
  if (tint_discard) {
    tint_discard_func();
    return;
  }
  value = inner_result;
  return;
}
