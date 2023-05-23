fn f_1() {
  var v = 0u;
  var n = 0u;
  var offset_1 = 0u;
  var count = 0u;
  let x_14 = v;
  let x_15 = n;
  let x_16 = offset_1;
  let x_17 = count;
  let x_12 = insertBits(x_14, x_15, x_16, x_17);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
