fn main_1() {
  let u1 : u32 = 10u;
  let i1 : i32 = 30;
  let v2u1 : vec2<u32> = vec2<u32>(10u, 20u);
  let v2i1 : vec2<i32> = vec2<i32>(30, 40);
  let x_1 : u32 = countOneBits(u1);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
