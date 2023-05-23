fn f_1() {
  var v = vec3i();
  var offset_1 = 0u;
  var count = 0u;
  let x_17 = v;
  let x_18 = offset_1;
  let x_19 = count;
  let x_15 = extractBits(x_17, x_18, x_19);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
