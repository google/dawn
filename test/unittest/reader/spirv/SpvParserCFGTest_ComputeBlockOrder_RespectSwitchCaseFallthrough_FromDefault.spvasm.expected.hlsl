static uint var_1 = 0u;

void main_1() {
  switch(42u) {
    case 20u: {
      break;
    }
    default: {
      /* fallthrough */
    }
    case 30u: {
      /* fallthrough */
    }
    case 40u: {
      break;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
