SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupAdd_2ab40a() -> vec4<f16> {
  var res : vec4<f16> = subgroupAdd(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_2ab40a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_2ab40a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/2ab40a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupAdd_2ab40a() -> vec4<f16> {
  var res : vec4<f16> = subgroupAdd(vec4<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_2ab40a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_2ab40a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/2ab40a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
