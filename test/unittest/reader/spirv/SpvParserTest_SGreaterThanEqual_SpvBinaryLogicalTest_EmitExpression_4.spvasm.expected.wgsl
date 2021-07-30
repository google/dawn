fn main_1() {
  let x_1 : vec2<bool> = (bitcast<vec2<i32>>(vec2<u32>(10u, 20u)) >= vec2<i32>(40, 30));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
