static uint var_1 = 0u;

void main_1() {
  switch(42u) {
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
