builtins/gen/smoothstep/5f615b.wgsl:28:24 warning: use of deprecated builtin
  var res: vec4<f32> = smoothStep(vec4<f32>(), vec4<f32>(), vec4<f32>());
                       ^^^^^^^^^^

fn smoothStep_5f615b() {
  var res : vec4<f32> = smoothStep(vec4<f32>(), vec4<f32>(), vec4<f32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothStep_5f615b();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  smoothStep_5f615b();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  smoothStep_5f615b();
}
