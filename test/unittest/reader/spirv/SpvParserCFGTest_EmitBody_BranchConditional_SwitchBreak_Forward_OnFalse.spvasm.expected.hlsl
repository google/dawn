static uint var_1 = 0u;

void main_1() {
  var_1 = 1u;
  switch(42u) {
    case 20u: {
      var_1 = 20u;
      if (false) {
        break;
      }
      var_1 = 30u;
      break;
    }
    default: {
      break;
    }
  }
  var_1 = 8u;
  return;
}

void main() {
  main_1();
  return;
}
