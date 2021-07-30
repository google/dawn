var<private> x_4 : i32;

var<private> x_1 : vec4<f32>;

fn main_1() {
  let x_2 : i32 = x_4;
  return;
}

struct main_out {
  [[builtin(position)]]
  x_1_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[builtin(instance_index)]] x_4_param : u32) -> main_out {
  x_4 = bitcast<i32>(x_4_param);
  main_1();
  return main_out(x_1);
}
