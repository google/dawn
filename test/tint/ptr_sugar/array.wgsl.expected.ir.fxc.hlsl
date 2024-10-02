
void deref_const() {
  int a[10] = (int[10])0;
  int b = a[int(0)];
  a[int(0)] = int(42);
}

void no_deref_const() {
  int a[10] = (int[10])0;
  int b = a[int(0)];
  a[int(0)] = int(42);
}

void deref_let() {
  int a[10] = (int[10])0;
  int i = int(0);
  int b = a[i];
  a[int(0)] = int(42);
}

void no_deref_let() {
  int a[10] = (int[10])0;
  int i = int(0);
  int b = a[i];
  a[int(0)] = int(42);
}

void deref_var() {
  int a[10] = (int[10])0;
  int i = int(0);
  int v = i;
  int b = a[v];
  a[int(0)] = int(42);
}

void no_deref_var() {
  int a[10] = (int[10])0;
  int i = int(0);
  int v_1 = i;
  int b = a[v_1];
  a[int(0)] = int(42);
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

