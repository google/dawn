#version 310 es
precision highp float;
precision highp int;

bool continue_execution = true;
layout(location = 1) flat in ivec3 tint_symbol_loc1_Input;
layout(location = 2) out int tint_symbol_loc2_Output;
int f(int x) {
  if ((x == 10)) {
    continue_execution = false;
  }
  return x;
}
int tint_symbol_inner(ivec3 x) {
  int y = x[0u];
  {
    while(true) {
      int r = f(y);
      if ((r == 0)) {
        break;
      }
      {
      }
      continue;
    }
  }
  if (!(continue_execution)) {
    discard;
  }
  return y;
}
void main() {
  tint_symbol_loc2_Output = tint_symbol_inner(tint_symbol_loc1_Input);
}
