[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared float a[10];
groupshared float b[20];

void f() {
  const float x = a[0];
  const float y = b[0];
}
