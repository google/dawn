static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  if (false) {
  } else {
    var_1 = 1u;
  }
  var_1 = 999u;
  return;
}

void main() {
  main_1();
  return;
}
