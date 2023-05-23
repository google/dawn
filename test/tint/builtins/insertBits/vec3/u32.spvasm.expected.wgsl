fn f_1() {
  var v = vec3u();
  var n = vec3u();
  var offset_1 = 0u;
  var count = 0u;
  let x_17 = v;
  let x_18 = n;
  let x_19 = offset_1;
  let x_20 = count;
  let x_15 = insertBits(x_17, x_18, x_19, x_20);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
