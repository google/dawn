@compute @workgroup_size(1u, 1u, 1u)
fn f() {
  var v : i32 = 0i;
  var n : i32 = 0i;
  var offset : u32 = 0u;
  var count : u32 = 0u;
  _ = insertBits(v, n, offset, count);
}
