var<private> var_1 : u32;

fn main_1() {
  var guard10 : bool = true;
  if (false) {
    if (true) {
      guard10 = false;
    }
    if (guard10) {
      guard10 = false;
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
