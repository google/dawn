var<private> var_1 : u32;

fn main_1() {
  if (false) {
    var_1 = 1u;
  }
  var_1 = 2u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
