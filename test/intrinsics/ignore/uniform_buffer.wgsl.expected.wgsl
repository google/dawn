intrinsics/ignore/uniform_buffer.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(u);
    ^^^^^^

intrinsics/ignore/uniform_buffer.wgsl:11:5 warning: use of deprecated intrinsic
    ignore(u.i);
    ^^^^^^

[[block]]
struct S {
  i : i32;
};

[[binding(0), group(0)]] var<uniform> u : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(u);
  ignore(u.i);
}
