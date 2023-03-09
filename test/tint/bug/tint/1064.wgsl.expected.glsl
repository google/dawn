#version 310 es
precision highp float;

void tint_symbol() {
  while (true) {
    if (false) {
    } else {
      break;
    }
    {
      if (false) { break; }
    }
  }
}

void main() {
  tint_symbol();
  return;
}
