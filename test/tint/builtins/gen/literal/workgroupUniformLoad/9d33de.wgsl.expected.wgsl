var<workgroup> arg_0 : i32;

fn workgroupUniformLoad_9d33de() {
  var res : i32 = workgroupUniformLoad(&(arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@compute @workgroup_size(1)
fn compute_main() {
  workgroupUniformLoad_9d33de();
}
