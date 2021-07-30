fn main_1() {
  let x_1 : f32 = 50.0;
  let x_2 : f32 = dpdyCoarse(x_1);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
