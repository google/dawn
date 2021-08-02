static uint x_1 = 0u;
static bool x_7 = false;
static bool x_8 = false;

void main_1() {
  uint x_41_phi = 0u;
  switch(1u) {
    default: {
      /* fallthrough */
      {
        /* fallthrough */
      }
      {
        if (true) {
        } else {
          x_41_phi = 0u;
          break;
        }
        x_41_phi = 1u;
      }
      break;
    }
    case 0u: {
      /* fallthrough */
      {
        if (true) {
        } else {
          x_41_phi = 0u;
          break;
        }
        x_41_phi = 1u;
      }
      break;
    }
    case 1u: {
      if (true) {
      } else {
        x_41_phi = 0u;
        break;
      }
      x_41_phi = 1u;
      break;
    }
  }
  const uint x_41 = x_41_phi;
  return;
}

void main() {
  main_1();
  return;
}
