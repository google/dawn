fn select_17441a() {
  var res = select(vec4(1.0), vec4(1.0), true);
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
