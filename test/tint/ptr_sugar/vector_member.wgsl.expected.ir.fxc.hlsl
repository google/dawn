
void deref() {
  int3 a = (int(0)).xxx;
  int3 p = a;
  int b = p.x;
  p[0u] = int(42);
}

void no_deref() {
  int3 a = (int(0)).xxx;
  int3 p = a;
  int b = p.x;
  p[0u] = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
}

