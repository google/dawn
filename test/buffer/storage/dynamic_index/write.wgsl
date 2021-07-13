struct Inner {
    a : vec3<i32>;
    b : i32;
    c : vec3<u32>;
    d : u32;
    e : vec3<f32>;
    f : f32;
    g : mat2x3<f32>;
    h : mat3x2<f32>;
    i : [[stride(16)]] array<vec4<i32>, 4>;
};

[[block]]
struct S {
    arr : array<Inner>;
};

[[binding(0), group(0)]] var<storage, read_write> s : S;

[[stage(compute), workgroup_size(1)]]
fn main([[builtin(local_invocation_index)]] idx : u32) {
    s.arr[idx].a = vec3<i32>();
    s.arr[idx].b = i32();
    s.arr[idx].c = vec3<u32>();
    s.arr[idx].d = u32();
    s.arr[idx].e = vec3<f32>();
    s.arr[idx].f = f32();
    s.arr[idx].g = mat2x3<f32>();
    s.arr[idx].h = mat3x2<f32>();
    s.arr[idx].i = [[stride(16)]] array<vec4<i32>, 4>();
}
