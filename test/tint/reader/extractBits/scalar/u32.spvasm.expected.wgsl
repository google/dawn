@compute @workgroup_size(1u, 1u, 1u)
fn f() {
  var v : u32 = 0u;
  var offset : u32 = 0u;
  var count : u32 = 0u;
  _ = extractBits(v, offset, count);
}
