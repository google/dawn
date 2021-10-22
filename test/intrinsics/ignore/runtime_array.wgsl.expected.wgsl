intrinsics/ignore/runtime_array.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(s.arr);
    ^^^^^^

[[block]]
struct S {
  arr : array<i32>;
};

[[binding(0), group(0)]] var<storage, read_write> s : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(s.arr);
}
