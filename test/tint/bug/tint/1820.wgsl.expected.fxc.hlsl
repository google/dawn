[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void foo(float x) {
  do {
  } while (false);
}

static int global = 0;

int baz(int x) {
  global = 42;
  return x;
}

void bar(float x) {
  baz(int(x));
  do {
  } while (false);
}

void main() {
}
