SKIP: FAILED


fn f_1() {
  var v = vec3i();
  var n = vec3i();
  var offset_1 = 0u;
  var count = 0u;
  let x_16 = insertBits(v, n, offset_1, count);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}

Failed to generate: :16:21 error: glsl.bitfieldInsert: no matching call to 'glsl.bitfieldInsert(vec3<i32>, vec3<i32>, i32, i32)'

1 candidate function:
 • 'glsl.bitfieldInsert(base: T  ✗ , insert: T  ✗ , offset: i32  ✓ , bits: i32  ✓ ) -> T' where:
      ✗  'T' is 'i32' or 'u32'

    %15:vec3<i32> = glsl.bitfieldInsert %6, %7, %13, %14
                    ^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f_1 = func():void {
  $B1: {
    %v:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(0i)
    %n:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(0i)
    %offset_1:ptr<function, u32, read_write> = var, 0u
    %count:ptr<function, u32, read_write> = var, 0u
    %6:vec3<i32> = load %v
    %7:vec3<i32> = load %n
    %8:u32 = load %offset_1
    %9:u32 = load %count
    %10:u32 = min %8, 32u
    %11:u32 = sub 32u, %10
    %12:u32 = min %9, %11
    %13:i32 = convert %10
    %14:i32 = convert %12
    %15:vec3<i32> = glsl.bitfieldInsert %6, %7, %13, %14
    %x_16:vec3<i32> = let %15
    ret
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %18:void = call %f_1
    ret
  }
}


tint executable returned error: exit status 1
