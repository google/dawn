static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  if (false) {
    var_1 = 1u;
  } else {
    var_1 = 2u;
  }
  var_1 = 999u;
  return;
}

void main() {
  main_1();
  return;
}
