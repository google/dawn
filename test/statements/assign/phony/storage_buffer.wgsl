[[block]]
struct S {
    i : i32;
};

[[binding(0), group(0)]] var<storage, read_write> s : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = s;
    _ = s.i;
}
