fn x_200(x_201 : ptr<function, vec2<f32>>) -> f32 {
  let x_212 : f32 = (*(x_201)).x;
  return x_212;
}

fn main_1() {
  var x_11 : vec2<f32>;
  let x_12 : f32 = x_200(&(x_11));
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
