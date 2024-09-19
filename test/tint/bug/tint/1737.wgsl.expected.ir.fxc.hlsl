
groupshared float a[10];
groupshared float b[20];
void f() {
  float x = a[int(0)];
  float y = b[int(0)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

