fn f_1() {
  var v = vec3i();
  var n = vec3i();
  var offset_1 = 0u;
  var count = 0u;
  let x_18 = v;
  let x_19 = n;
  let x_20 = offset_1;
  let x_21 = count;
  let x_16 = insertBits(x_18, x_19, x_20, x_21);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
