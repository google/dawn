fn select_17441a() {
  const arg_0 = vec4(1.0);
  const arg_1 = vec4(1.0);
  var arg_2 = true;
  var res = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_17441a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_17441a();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_17441a();
}
