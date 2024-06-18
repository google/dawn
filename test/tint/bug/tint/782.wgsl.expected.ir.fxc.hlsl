SKIP: FAILED

void foo() {
  int[2] explicitStride = (int[2])0;
  int[2] implictStride = (int[2])0;
  implictStride = explicitStride;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

