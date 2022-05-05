fn f_1() {
  var v : i32 = 0i;
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_16 : i32 = v;
  let x_17 : u32 = offset_1;
  let x_18 : u32 = count;
  let x_14 : i32 = extractBits(x_16, x_17, x_18);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
