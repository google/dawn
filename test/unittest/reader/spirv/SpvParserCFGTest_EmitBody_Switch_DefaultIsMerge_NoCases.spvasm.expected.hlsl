static uint var_1 = 0u;

void main_1() {
  var_1 = 1u;
  switch(42u) {
    default: {
      break;
    }
  }
  var_1 = 7u;
  return;
}

void main() {
  main_1();
  return;
}
