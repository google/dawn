fn foo() -> f32 {
  let oob = 99;
  let b = vec4<f32>()[min(u32(oob), 3u)];
  var v : vec4<f32>;
  v[min(u32(oob), 3u)] = b;
  return b;
}
