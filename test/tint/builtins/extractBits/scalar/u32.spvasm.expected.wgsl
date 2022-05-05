fn f_1() {
  var v : u32 = 0u;
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_13 : u32 = v;
  let x_14 : u32 = offset_1;
  let x_15 : u32 = count;
  let x_11 : u32 = extractBits(x_13, x_14, x_15);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
