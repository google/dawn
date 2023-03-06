var<workgroup> arg_0 : f32;

fn workgroupUniformLoad_7a857c() {
  var res : f32 = workgroupUniformLoad(&(arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@compute @workgroup_size(1)
fn compute_main() {
  workgroupUniformLoad_7a857c();
}
