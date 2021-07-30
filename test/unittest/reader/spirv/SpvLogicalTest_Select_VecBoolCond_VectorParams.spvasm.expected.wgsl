fn main_1() {
  let x_1 : vec2<u32> = select(vec2<u32>(20u, 10u), vec2<u32>(10u, 20u), vec2<bool>(true, false));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
