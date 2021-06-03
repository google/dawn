struct Interface {
  [[location(1)]] col1 : f32;
  [[location(2)]] col2 : f32;
  [[builtin(position)]] pos : vec4<f32>;
};

[[stage(vertex)]]
fn vert_main() -> Interface {
  return Interface(0.4, 0.6, vec4<f32>());
}

[[stage(fragment)]]
fn frag_main(colors : Interface) {
  let r : f32 = colors.col1;
  let g : f32 = colors.col2;
}
