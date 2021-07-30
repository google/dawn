var<private> gl_Position : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);

fn main_1() {
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
