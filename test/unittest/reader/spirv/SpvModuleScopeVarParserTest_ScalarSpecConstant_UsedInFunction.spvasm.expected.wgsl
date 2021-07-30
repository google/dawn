let myconst : f32 = 2.5;

fn x_100() -> f32 {
  return (myconst + myconst);
}

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
