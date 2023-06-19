fn main_1() {
  var v = vec3f();
  let x_14 = v.y;
  let x_16 = v;
  let x_17 = x_16.xz;
  let x_18 = v;
  let x_19 = x_18.xzy;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
