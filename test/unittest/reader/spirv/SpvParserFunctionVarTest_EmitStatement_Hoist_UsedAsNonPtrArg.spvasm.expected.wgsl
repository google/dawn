struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn x_500(x_501 : i32) {
  return;
}

fn main_1() {
  var x_200 : i32;
  if (true) {
    x_200 = 1;
  } else {
    return;
  }
  x_500(x_200);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
