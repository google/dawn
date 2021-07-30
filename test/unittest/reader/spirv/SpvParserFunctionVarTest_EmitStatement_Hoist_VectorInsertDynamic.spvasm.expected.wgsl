struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var x_200 : vec2<i32>;
  if (true) {
    x_200 = vec2<i32>(0, 0);
    x_200[1] = 3;
  } else {
    return;
  }
  let x_201 : vec2<i32> = x_200;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
