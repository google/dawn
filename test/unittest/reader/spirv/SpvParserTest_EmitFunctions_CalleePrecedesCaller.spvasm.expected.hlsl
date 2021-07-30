uint leaf() {
  return 0u;
}

uint tint_symbol() {
  const uint leaf_result = leaf();
  return leaf_result;
}

void root() {
  const uint branch_result = tint_symbol();
  return;
}

void x_100_1() {
  return;
}

void x_100() {
  x_100_1();
  return;
}
