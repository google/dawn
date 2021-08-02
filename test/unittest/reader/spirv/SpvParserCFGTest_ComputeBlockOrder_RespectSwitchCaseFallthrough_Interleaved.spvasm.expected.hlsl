static uint var_1 = 0u;

void main_1() {
  switch(42u) {
    case 30u: {
      /* fallthrough */
      {
      }
      break;
    }
    case 50u: {
      break;
    }
    case 20u: {
      /* fallthrough */
      {
      }
      break;
    }
    case 40u: {
      break;
    }
    default: {
      break;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
