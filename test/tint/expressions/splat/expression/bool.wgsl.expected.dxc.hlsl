[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  bool tint_tmp = true;
  if (!tint_tmp) {
    tint_tmp = false;
  }
  bool2 v2 = bool2(((tint_tmp)).xx);
  bool tint_tmp_1 = true;
  if (!tint_tmp_1) {
    tint_tmp_1 = false;
  }
  bool3 v3 = bool3(((tint_tmp_1)).xxx);
  bool tint_tmp_2 = true;
  if (!tint_tmp_2) {
    tint_tmp_2 = false;
  }
  bool4 v4 = bool4(((tint_tmp_2)).xxxx);
}
