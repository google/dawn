var<workgroup> arg_0 : u32;

fn workgroupUniformLoad_37307c() {
  var res : u32 = workgroupUniformLoad(&(arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  workgroupUniformLoad_37307c();
}
