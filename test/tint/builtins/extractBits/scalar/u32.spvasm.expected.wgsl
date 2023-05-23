fn f_1() {
  var v = 0u;
  var offset_1 = 0u;
  var count = 0u;
  let x_13 = v;
  let x_14 = offset_1;
  let x_15 = count;
  let x_11 = extractBits(x_13, x_14, x_15);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
