fn f_1() {
  var v : vec3i = vec3i();
  var n : vec3i = vec3i();
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_18 : vec3i = v;
  let x_19 : vec3i = n;
  let x_20 : u32 = offset_1;
  let x_21 : u32 = count;
  let x_16 : vec3i = insertBits(x_18, x_19, x_20, x_21);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
