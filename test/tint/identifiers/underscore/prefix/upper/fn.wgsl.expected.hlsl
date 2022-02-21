[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void A() {
}

void _A() {
}

void B() {
  A();
}

void _B() {
  _A();
}
