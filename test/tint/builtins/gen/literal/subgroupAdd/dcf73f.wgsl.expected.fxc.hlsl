SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupAdd_dcf73f() -> vec2<f32> {
  var res : vec2<f32> = subgroupAdd(vec2<f32>(1.0f));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_dcf73f();
}

Failed to generate: error: Unknown builtin method: subgroupAdd
