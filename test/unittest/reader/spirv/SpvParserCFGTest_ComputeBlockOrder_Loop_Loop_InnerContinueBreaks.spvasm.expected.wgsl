var<private> var_1 : u32;

fn main_1() {
  loop {
    if (false) {
    } else {
      break;
    }
    loop {
      if (true) {
      } else {
        break;
      }

      continuing {
        if (false) {
        } else {
          break;
        }
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
