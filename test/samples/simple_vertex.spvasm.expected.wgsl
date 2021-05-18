[[builtin(position)]] var<out> gl_Position : vec4<f32>;

[[stage(vertex)]]
fn main() {
  gl_Position = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  return;
}
