var<private> var_1 : u32;

fn main_1() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
      if (true) {
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
