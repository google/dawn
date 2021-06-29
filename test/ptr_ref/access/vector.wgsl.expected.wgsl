[[stage(compute), workgroup_size(1)]]
fn main() {
  var v : vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
  let f : ptr<function, f32> = &(v.y);
  *(f) = 5.0;
}
