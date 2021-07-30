fn main_1() {
  let x_1 : vec2<i32> = (vec2<i32>(30, 40) / bitcast<vec2<i32>>(vec2<u32>(10u, 20u)));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
