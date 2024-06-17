SKIP: FAILED

uint leaf() {
  return 0u;
}

uint tint_symbol() {
  uint leaf_result = leaf();
  return leaf_result;
}

void root() {
  uint branch_result = tint_symbol();
}

void x_100_1() {
}

void x_100() {
  x_100_1();
}

