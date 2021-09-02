[[block]]
struct SSBO {
  m : [[stride(16)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<storage, read_write> ssbo : SSBO;

fn arr_to_mat2x2_stride_16(arr : [[stride(16)]] array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

fn mat2x2_stride_16_to_arr(mat : mat2x2<f32>) -> [[stride(16)]] array<vec2<f32>, 2u> {
  return [[stride(16)]] array<vec2<f32>, 2u>(mat[0u], mat[1u]);
}

fn f_1() {
  let x_15 : mat2x2<f32> = arr_to_mat2x2_stride_16(ssbo.m);
  ssbo.m = mat2x2_stride_16_to_arr(x_15);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn f() {
  f_1();
}
