
void foo(bool a, bool b, bool c, bool d, bool e) {
  if (a) {
    if (b) {
      return;
    }
    if (c) {
      if (d) {
        return;
      }
      if (e) {
      }
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  foo(true, true, false, false, false);
}

