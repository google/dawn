
static uint2 u = (1u).xx;
void f() {
  bool2 v = bool2(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

