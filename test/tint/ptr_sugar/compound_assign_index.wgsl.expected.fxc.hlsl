
void deref() {
  int3 a = (int(0)).xxx;
  a.x = asint((asuint(a.x) + 42u));
}

void no_deref() {
  int3 a = (int(0)).xxx;
  a.x = asint((asuint(a.x) + 42u));
}

void deref_inc() {
  int3 a = (int(0)).xxx;
  a.x = asint((asuint(a.x) + 1u));
}

void no_deref_inc() {
  int3 a = (int(0)).xxx;
  a.x = asint((asuint(a.x) + 1u));
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}

