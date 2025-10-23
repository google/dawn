fn get_bool() -> bool {
  return true;
}

@compute @workgroup_size(1)
fn f() {
  var v2 : vec2<bool> = vec2<bool>(get_bool());
  var v3 : vec3<bool> = vec3<bool>(get_bool());
  var v4 : vec4<bool> = vec4<bool>(get_bool());
}
