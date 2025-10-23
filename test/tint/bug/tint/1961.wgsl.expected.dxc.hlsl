
[numthreads(1, 1, 1)]
void f() {
  bool x = false;
  bool y = false;
  bool v = false;
  if (x) {
    v = true;
  } else {
    v = false;
  }
  if (v) {
  }
}

