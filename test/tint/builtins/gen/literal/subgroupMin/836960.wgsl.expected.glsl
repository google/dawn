SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupMin_836960() -> vec3<f32> {
  var res : vec3<f32> = subgroupMin(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_836960();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_836960();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/836960.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupMin_836960() -> vec3<f32> {
  var res : vec3<f32> = subgroupMin(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_836960();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_836960();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/836960.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
