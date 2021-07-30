var<private> var_1 : u32;

fn main_1() {
  if (false) {
    if (false) {
    }
  } else {
    if (false) {
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
