[[stage(compute), workgroup_size(1)]]
fn f() {
  let a : vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
  let b : vec3<i32> = bitcast<vec3<i32>>(a);
}
