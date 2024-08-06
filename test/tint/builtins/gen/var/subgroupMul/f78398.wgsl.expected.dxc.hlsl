SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupMul_f78398() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = subgroupMul(arg_0);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_f78398();
}

Failed to generate: error: Unknown builtin method: subgroupMul
