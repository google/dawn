// flags: --hlsl-shader-model 62
enable f16;

@group(0) @binding(0) var<storage, read_write> s: array<u32>;

@compute @workgroup_size(1u)
fn main(){
    const kArray = array(
                    mat3x2(vec2(0.0h,1.0h),vec2(2.0h,3.0h),vec2(2.0h,3.0h)),
                     mat3x2(vec2(0.0h,1.0h),vec2(2.0h,3.0h),vec2(2.0h,3.0h)),
                    );
    var q = 0u;
    s[0] = u32(kArray[q][0][0]);
}
