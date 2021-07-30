var<private> var_1 : u32;

fn main_1() {
  if (false) {
    loop {
      if (false) {
      } else {
        break;
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
