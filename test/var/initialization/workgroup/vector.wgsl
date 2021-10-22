var<workgroup> v : vec3<i32>;

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = v;
}
