struct Interface {
  [[location(0)]]
  i : i32;
  [[location(1)]]
  u : u32;
  [[location(2)]]
  vi : vec4<i32>;
  [[location(3)]]
  vu : vec4<u32>;
  [[builtin(position)]]
  pos : vec4<f32>;
};

[[stage(vertex)]]
fn vert_main() -> Interface {
  return Interface();
}

[[stage(fragment)]]
fn frag_main(inputs : Interface) -> [[location(0)]] i32 {
  return inputs.i;
}
