[[block]]
struct Constants {
  zero : u32;
};

[[group(1), binding(0)]] var<uniform> constants : Constants;

[[block]]
struct Result {
  value : u32;
};

[[group(1), binding(1)]] var<storage, write> result : Result;

[[block]]
struct TestData {
  data : array<atomic<i32>, 3>;
};

[[group(0), binding(0)]] var<storage, read_write> s : TestData;

fn runTest() -> i32 {
  return atomicLoad(&(s.data[(0u + u32(constants.zero))]));
}

[[stage(compute), workgroup_size(1)]]
fn main() {
  result.value = u32(runTest());
}
