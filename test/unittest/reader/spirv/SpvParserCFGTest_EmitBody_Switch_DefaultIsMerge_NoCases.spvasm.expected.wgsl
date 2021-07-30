var<private> var_1 : u32;

fn main_1() {
  var_1 = 1u;
  switch(42u) {
    default: {
    }
  }
  var_1 = 7u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
