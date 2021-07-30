fn main_1() {
  let x_1 : u32 = bitcast<u32>((bitcast<i32>(10u) >> bitcast<u32>(30)));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
