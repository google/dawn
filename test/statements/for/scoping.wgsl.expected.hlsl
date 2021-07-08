[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    for(int must_not_collide = 0; ; ) {
    }
  }
  int must_not_collide = 0;
}
