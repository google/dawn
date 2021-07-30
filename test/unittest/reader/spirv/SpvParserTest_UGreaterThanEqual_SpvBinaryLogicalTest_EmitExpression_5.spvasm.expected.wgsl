fn main_1() {
  let x_1 : vec2<bool> = (vec2<u32>(10u, 20u) >= bitcast<vec2<u32>>(vec2<i32>(40, 30)));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
