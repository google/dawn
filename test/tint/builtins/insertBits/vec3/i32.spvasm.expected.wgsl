fn f_1() {
  var v : vec3<i32> = vec3<i32>();
  var n : vec3<i32> = vec3<i32>();
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_18 : vec3<i32> = v;
  let x_19 : vec3<i32> = n;
  let x_20 : u32 = offset_1;
  let x_21 : u32 = count;
  let x_16 : vec3<i32> = insertBits(x_18, x_19, x_20, x_21);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
