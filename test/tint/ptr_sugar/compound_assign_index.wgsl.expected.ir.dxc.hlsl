
void deref() {
  int3 a = (0).xxx;
  int3 p = a;
  p[0] = (p.x + 42);
}

void no_deref() {
  int3 a = (0).xxx;
  int3 p = a;
  p[0] = (p.x + 42);
}

void deref_inc() {
  int3 a = (0).xxx;
  int3 p = a;
  p[0] = (p.x + 1);
}

void no_deref_inc() {
  int3 a = (0).xxx;
  int3 p = a;
  p[0] = (p.x + 1);
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}

