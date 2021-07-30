var<private> var_1 : u32;

fn main_1() {
  loop {

    continuing {
      var_1 = 1u;
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
