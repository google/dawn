fn f_1() {
  var v : vec3<i32> = vec3<i32>();
  var offset_1 : u32 = 0u;
  var count : u32 = 0u;
  let x_17 : vec3<i32> = v;
  let x_18 : u32 = offset_1;
  let x_19 : u32 = count;
  let x_15 : vec3<i32> = extractBits(x_17, x_18, x_19);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
