SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupMin_7def0a() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_7def0a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_7def0a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/7def0a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupMin_7def0a() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_7def0a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_7def0a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/7def0a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
