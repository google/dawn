fn get_u32() -> u32 {
  return 1u;
}

@compute @workgroup_size(1)
fn f() {
  var v2 : vec2<u32> = vec2<u32>(get_u32());
  var v3 : vec3<u32> = vec3<u32>(get_u32());
  var v4 : vec4<u32> = vec4<u32>(get_u32());
}
