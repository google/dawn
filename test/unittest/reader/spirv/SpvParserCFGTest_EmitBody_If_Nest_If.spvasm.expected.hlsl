static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  if (false) {
    var_1 = 1u;
    if (true) {
      var_1 = 2u;
    }
    var_1 = 3u;
  } else {
    var_1 = 4u;
    if (true) {
    } else {
      var_1 = 5u;
    }
    var_1 = 6u;
  }
  var_1 = 999u;
  return;
}

void main() {
  main_1();
  return;
}
