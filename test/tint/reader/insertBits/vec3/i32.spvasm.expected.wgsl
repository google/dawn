@compute @workgroup_size(1u, 1u, 1u)
fn f() {
  var v : vec3<i32> = vec3<i32>();
  var n : vec3<i32> = vec3<i32>();
  var offset : u32 = 0u;
  var count : u32 = 0u;
  _ = insertBits(v, n, offset, count);
}
