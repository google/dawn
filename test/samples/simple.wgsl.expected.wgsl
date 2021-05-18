fn bar() {
}

[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
  var a : vec2<f32> = vec2<f32>();
  bar();
  return vec4<f32>(0.400000006, 0.400000006, 0.800000012, 1.0);
}
