[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    int must_not_collide = 0;
    while (true) {
    }
  }
  int must_not_collide = 0;
}
