SKIP: FAILED

void comp_main_1() {
}

[numthreads(2, 4, 8)]
void comp_main() {
  comp_main_1();
}

