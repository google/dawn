fn main_1() {
  let x_1 : vec2<i32> = (vec2<i32>(40, 30) >> vec2<u32>(20u, 10u));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
