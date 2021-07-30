var<private> var_1 : u32;

fn main_1() {
  var_1 = 0u;
  if (false) {
  }
  var_1 = 5u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
