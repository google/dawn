[[block]]
struct UBO {
  data: [[stride(16)]] array<i32, 4>;
  dynamic_idx: i32;
};
[[group(0), binding(0)]] var<uniform> ubo: UBO;
[[block]]
struct Result {
  out: i32;
};
[[group(0), binding(2)]] var<storage, read_write> result: Result;

[[stage(compute), workgroup_size(1)]]
fn f() {
  result.out = ubo.data[ubo.dynamic_idx];
}
