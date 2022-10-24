bug/tint/1064.wgsl:12:9 warning: use of deprecated language feature: `break` must not be used to exit from a continuing block. Use `break-if` instead.
        break;
        ^^^^^

#version 310 es
precision mediump float;

void tint_symbol() {
  while (true) {
    if (false) {
    } else {
      break;
    }
    {
      if (true) {
      } else {
        break;
      }
    }
  }
}

void main() {
  tint_symbol();
  return;
}
