fn f_1() {
  var v : u32 = 0u;
  var n : u32 = 0u;
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_14 : u32 = v;
  let x_15 : u32 = n;
  let x_16 : u32 = offset_1;
  let x_17 : u32 = count;
  let x_12 : u32 = insertBits(x_14, x_15, x_16, x_17);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
