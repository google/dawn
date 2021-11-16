intrinsics/ignore/call.wgsl:7:5 warning: use of deprecated intrinsic
    ignore(f(1, 2, 3));
    ^^^^^^

#version 310 es
precision mediump float;

int f(int a, int b, int c) {
  return ((a * b) + c);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  f(1, 2, 3);
  return;
}
void main() {
  tint_symbol();
}


