var<private> gl_Position : vec4<f32>;

fn main_1() {
  gl_Position = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  return;
}

struct main_out {
  [[builtin(position)]]
  gl_Position : vec4<f32>;
};

[[stage(vertex)]]
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
