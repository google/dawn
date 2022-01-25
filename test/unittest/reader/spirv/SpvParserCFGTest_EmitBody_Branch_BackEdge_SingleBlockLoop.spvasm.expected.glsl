SKIP: FAILED


var<private> var_1 : u32;

fn main_1() {
  loop {
    var_1 = 1u;
  }
  return;
}

@stage(fragment)
fn main() {
  main_1();
}

error: loop does not exit
