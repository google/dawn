var<private> var_1 : u32;

fn main_1() {
  var guard10 : bool = true;
  if (false) {
    guard10 = false;
  } else {
    if (guard10) {
      if (true) {
        guard10 = false;
      }
      if (guard10) {
        guard10 = false;
      }
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
