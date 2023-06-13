[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  bool x = false;
  bool y = false;
  bool tint_tmp = x;
  if (tint_tmp) {
    bool tint_tmp_1 = true;
    if (!tint_tmp_1) {
      tint_tmp_1 = y;
    }
    tint_tmp = (tint_tmp_1);
  }
  if ((tint_tmp)) {
  }
}
