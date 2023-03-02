SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  while (true) {
    var_1 = 1u;
    if (false) {
      break;
    }
  }
  var_1 = 999u;
  return;
}

void main() {
  main_1();
  return;
}
