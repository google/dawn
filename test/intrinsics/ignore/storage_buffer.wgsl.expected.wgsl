intrinsics/ignore/storage_buffer.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(s);
    ^^^^^^

intrinsics/ignore/storage_buffer.wgsl:11:5 warning: use of deprecated intrinsic
    ignore(s.i);
    ^^^^^^

[[block]]
struct S {
  i : i32;
};

[[binding(0), group(0)]] var<storage, read_write> s : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(s);
  ignore(s.i);
}
