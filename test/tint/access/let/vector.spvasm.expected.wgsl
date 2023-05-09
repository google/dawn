const x_10 = vec3f(1.0f, 2.0f, 3.0f);

fn main_1() {
  let x_11 : f32 = x_10.y;
  let x_13 : vec2f = vec2f(x_10.x, x_10.z);
  let x_14 : vec3f = vec3f(x_10.x, x_10.z, x_10.y);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
