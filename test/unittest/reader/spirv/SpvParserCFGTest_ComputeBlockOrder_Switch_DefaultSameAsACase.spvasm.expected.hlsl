static uint var_1 = 0u;

void main_1() {
  switch(42u) {
    case 40u: {
      break;
    }
    case 20u: {
      break;
    }
    default: {
      /* fallthrough */
      {
      }
      break;
    }
    case 30u: {
      break;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
