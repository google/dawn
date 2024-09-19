
void f() {
  {
    int must_not_collide = int(0);
    while(true) {
      break;
    }
  }
  int must_not_collide = int(0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

