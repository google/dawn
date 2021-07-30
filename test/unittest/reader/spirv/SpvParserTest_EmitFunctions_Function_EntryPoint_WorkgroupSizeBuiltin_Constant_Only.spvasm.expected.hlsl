void comp_main_1() {
  return;
}

[numthreads(3, 5, 7)]
void comp_main() {
  comp_main_1();
  return;
}
