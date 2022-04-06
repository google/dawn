SKIP: FAILED


struct S {
  field0 : u32,
  field1 : f32,
  field2 : array<u32, 2u>,
};

fn main_1() {
  var x_25 : u32;
  let x_2 : u32 = (1u + 1u);
  x_25 = 1u;
  loop {

    continuing {
      x_25 = x_2;
    }
  }
  x_25 = 2u;
  return;
}

@stage(fragment)
fn main() {
  main_1();
}

error: loop does not exit
