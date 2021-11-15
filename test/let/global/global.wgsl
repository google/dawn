struct MyStruct {
    f1 : f32;
};

type MyArray = array<f32, 10>;

// Global lets
let v1 = 1;
let v2 = 1u;
let v3 = 1.0;

let v4 = vec3<i32>(1, 1, 1);
let v5 = vec3<u32>(1u, 1u, 1u);
let v6 = vec3<f32>(1.0, 1.0, 1.0);

let v7 = mat3x3<f32>(vec3<f32>(1.0, 1.0, 1.0), vec3<f32>(1.0, 1.0, 1.0), vec3<f32>(1.0, 1.0, 1.0));

let v8 = MyStruct();
let v9 = MyArray();

[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
    return vec4<f32>(0.0,0.0,0.0,0.0);
}
