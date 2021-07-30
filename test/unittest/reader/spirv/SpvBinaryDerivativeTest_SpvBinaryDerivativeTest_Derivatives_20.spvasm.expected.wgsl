fn main_1() {
  let x_1 : vec3<f32> = vec3<f32>(50.0, 60.0, 70.0);
  let x_2 : vec3<f32> = dpdxCoarse(x_1);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
