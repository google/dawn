@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

var<workgroup> arg_0 : bool;

fn workgroupUniformLoad_b75d53() -> i32 {
  var res : bool = workgroupUniformLoad(&(arg_0));
  return select(0, 1, all((res == bool())));
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = workgroupUniformLoad_b75d53();
}
