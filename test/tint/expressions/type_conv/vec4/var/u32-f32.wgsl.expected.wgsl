var<private> u = vec4<u32>(1u);

@compute @workgroup_size(1)
fn f() {
  let v : vec4<f32> = vec4<f32>(u);
}
