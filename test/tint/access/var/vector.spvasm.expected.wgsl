fn main_1() {
  var v = vec3f();
  let x_14 = v.y;
  let x_16 = v;
  let x_17 = vec2f(x_16.x, x_16.z);
  let x_18 = v;
  let x_19 = vec3f(x_18.x, x_18.z, x_18.y);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
