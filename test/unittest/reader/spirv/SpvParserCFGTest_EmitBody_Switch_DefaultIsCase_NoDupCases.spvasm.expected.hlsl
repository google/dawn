static uint var_1 = 0u;

void main_1() {
  var_1 = 1u;
  switch(42u) {
    case 40u: {
      var_1 = 40u;
      break;
    }
    case 20u: {
      var_1 = 20u;
      break;
    }
    default: {
      var_1 = 30u;
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
