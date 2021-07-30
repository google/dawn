void comp_main_1() {
  return;
}

[numthreads(2, 4, 8)]
void comp_main() {
  comp_main_1();
  return;
}
