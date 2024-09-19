struct S {
  int x;
};


void deref() {
  S a = (S)0;
  S p = a;
  int b = p.x;
  p.x = int(42);
}

void no_deref() {
  S a = (S)0;
  S p = a;
  int b = p.x;
  p.x = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
}

