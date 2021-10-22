intrinsics/ignore/call.wgsl:7:5 warning: use of deprecated intrinsic
    ignore(f(1, 2, 3));
    ^^^^^^

int f(int a, int b, int c) {
  return ((a * b) + c);
}

[numthreads(1, 1, 1)]
void main() {
  f(1, 2, 3);
  return;
}
