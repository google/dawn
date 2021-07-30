var<private> var_1 : u32;

fn main_1() {
  if (false) {
    discard;
  }
  discard;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
