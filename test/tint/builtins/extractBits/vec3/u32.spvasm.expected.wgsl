fn f_1() {
  var v = vec3u();
  var offset_1 = 0u;
  var count = 0u;
  let x_16 = v;
  let x_17 = offset_1;
  let x_18 = count;
  let x_14 = extractBits(x_16, x_17, x_18);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
