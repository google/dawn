
uint f() {
  return 2147483648u;
}

[numthreads(1, 1, 1)]
void main() {
  f();
}

