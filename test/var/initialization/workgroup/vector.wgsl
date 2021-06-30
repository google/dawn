var<workgroup> v : vec3<i32>;

[[stage(compute), workgroup_size(1)]]
fn main() {
    ignore(v);
}
