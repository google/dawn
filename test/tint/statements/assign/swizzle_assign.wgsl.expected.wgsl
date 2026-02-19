struct Buf {
  res : vec4u,
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1)
fn main() {
  var v = vec4u(1, 2, 3, 4);
  v.zy = vec2(5, 6);
  v.yz.yx = vec2(99, 100);
  v.yz.y = 200;
  v.rgb *= vec3(100);
  let p = &(buf.res);
  p.xyzw = vec4u(0);
  p.zy = vec2(1, 2);
  p.yz.yx = vec2(3, 4);
  p.yz.y = 5;
  p.rgb += vec3(10);
}
