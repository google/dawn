[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    [loop] for(int must_not_collide = 0; ; ) {
      break;
    }
  }
  int must_not_collide = 0;
}
