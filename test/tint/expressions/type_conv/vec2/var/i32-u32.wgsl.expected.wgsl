var<private> u = vec2<i32>(1i);

@compute @workgroup_size(1)
fn f() {
  let v : vec2<u32> = vec2<u32>(u);
}
