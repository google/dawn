
void deref_const() {
  int3 a = (0).xxx;
  int3 p = a;
  int b = p.x;
  p[0] = 42;
}

void no_deref_const() {
  int3 a = (0).xxx;
  int3 p = a;
  int b = p.x;
  p[0] = 42;
}

void deref_let() {
  int3 a = (0).xxx;
  int3 p = a;
  int i = 0;
  int b = p[i];
  p[0] = 42;
}

void no_deref_let() {
  int3 a = (0).xxx;
  int3 p = a;
  int i = 0;
  int b = p[i];
  p[0] = 42;
}

void deref_var() {
  int3 a = (0).xxx;
  int3 p = a;
  int i = 0;
  int b = p[i];
  p[0] = 42;
}

void no_deref_var() {
  int3 a = (0).xxx;
  int3 p = a;
  int i = 0;
  int b = p[i];
  p[0] = 42;
}

[numthreads(1, 1, 1)]
void main() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
}

