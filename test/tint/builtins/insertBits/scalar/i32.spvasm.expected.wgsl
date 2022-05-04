fn f_1() {
  var v : i32 = 0i;
  var n : i32 = 0i;
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_17 : i32 = v;
  let x_18 : i32 = n;
  let x_19 : u32 = offset_1;
  let x_20 : u32 = count;
  let x_15 : i32 = insertBits(x_17, x_18, x_19, x_20);
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn f() {
  f_1();
}
