[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float a = (1.0f).xx.y;
  float b = (1.0f).xxx.z;
  float c = (1.0f).xxxx.w;
}
