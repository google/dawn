static uint var_1 = 0u;

void main_1() {
  var_1 = 1u;
  bool guard10 = true;
  if (false) {
    var_1 = 2u;
    if (true) {
      guard10 = false;
    }
    if (guard10) {
      var_1 = 3u;
      guard10 = false;
    }
  } else {
    if (guard10) {
      var_1 = 4u;
      guard10 = false;
    }
  }
  var_1 = 5u;
  return;
}

void main() {
  main_1();
  return;
}
