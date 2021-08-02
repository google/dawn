static uint var_1 = 0u;

void main_1() {
  switch(42u) {
    case 20u: {
      if (false) {
        break;
      }
      break;
    }
    default: {
      /* fallthrough */
      {
        if (false) {
        } else {
          break;
        }
      }
      break;
    }
    case 50u: {
      if (false) {
      } else {
        break;
      }
      break;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
