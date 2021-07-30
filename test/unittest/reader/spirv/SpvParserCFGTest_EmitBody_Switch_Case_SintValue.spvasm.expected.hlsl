static uint var_1 = 0u;

void main_1() {
  var_1 = 1u;
  switch(42) {
    case -294967296: {
      var_1 = 40u;
      break;
    }
    case 2000000000: {
      var_1 = 30u;
      break;
    }
    case 20: {
      var_1 = 20u;
      break;
    }
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
