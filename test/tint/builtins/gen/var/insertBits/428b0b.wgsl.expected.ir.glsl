SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn insertBits_428b0b() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var arg_1 = vec3<i32>(1i);
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec3<i32> = insertBits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<i32>,
}

@fragment
fn fragment_main() {
  prevent_dce = insertBits_428b0b();
}

Failed to generate: :24:21 error: glsl.bitfieldInsert: no matching call to 'glsl.bitfieldInsert(vec3<i32>, vec3<i32>, i32, i32)'

1 candidate function:
 • 'glsl.bitfieldInsert(base: T  ✗ , insert: T  ✗ , offset: i32  ✓ , bits: i32  ✓ ) -> T' where:
      ✗  'T' is 'i32' or 'u32'

    %16:vec3<i32> = glsl.bitfieldInsert %7, %8, %14, %15
                    ^^^^^^^^^^^^^^^^^^^

:10:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
tint_symbol_1 = struct @align(16), @block {
  tint_symbol:vec3<i32> @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
}

%insertBits_428b0b = func():vec3<i32> {
  $B2: {
    %arg_0:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(1i)
    %arg_1:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(1i)
    %arg_2:ptr<function, u32, read_write> = var, 1u
    %arg_3:ptr<function, u32, read_write> = var, 1u
    %7:vec3<i32> = load %arg_0
    %8:vec3<i32> = load %arg_1
    %9:u32 = load %arg_2
    %10:u32 = load %arg_3
    %11:u32 = min %9, 32u
    %12:u32 = sub 32u, %11
    %13:u32 = min %10, %12
    %14:i32 = convert %11
    %15:i32 = convert %13
    %16:vec3<i32> = glsl.bitfieldInsert %7, %8, %14, %15
    %res:ptr<function, vec3<i32>, read_write> = var, %16
    %18:vec3<i32> = load %res
    ret %18
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %20:vec3<i32> = call %insertBits_428b0b
    %21:ptr<storage, vec3<i32>, read_write> = access %1, 0u
    store %21, %20
    ret
  }
}


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn insertBits_428b0b() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var arg_1 = vec3<i32>(1i);
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec3<i32> = insertBits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<i32>,
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = insertBits_428b0b();
}

Failed to generate: :24:21 error: glsl.bitfieldInsert: no matching call to 'glsl.bitfieldInsert(vec3<i32>, vec3<i32>, i32, i32)'

1 candidate function:
 • 'glsl.bitfieldInsert(base: T  ✗ , insert: T  ✗ , offset: i32  ✓ , bits: i32  ✓ ) -> T' where:
      ✗  'T' is 'i32' or 'u32'

    %16:vec3<i32> = glsl.bitfieldInsert %7, %8, %14, %15
                    ^^^^^^^^^^^^^^^^^^^

:10:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
tint_symbol_1 = struct @align(16), @block {
  tint_symbol:vec3<i32> @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
}

%insertBits_428b0b = func():vec3<i32> {
  $B2: {
    %arg_0:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(1i)
    %arg_1:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(1i)
    %arg_2:ptr<function, u32, read_write> = var, 1u
    %arg_3:ptr<function, u32, read_write> = var, 1u
    %7:vec3<i32> = load %arg_0
    %8:vec3<i32> = load %arg_1
    %9:u32 = load %arg_2
    %10:u32 = load %arg_3
    %11:u32 = min %9, 32u
    %12:u32 = sub 32u, %11
    %13:u32 = min %10, %12
    %14:i32 = convert %11
    %15:i32 = convert %13
    %16:vec3<i32> = glsl.bitfieldInsert %7, %8, %14, %15
    %res:ptr<function, vec3<i32>, read_write> = var, %16
    %18:vec3<i32> = load %res
    ret %18
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %20:vec3<i32> = call %insertBits_428b0b
    %21:ptr<storage, vec3<i32>, read_write> = access %1, 0u
    store %21, %20
    ret
  }
}


fn insertBits_428b0b() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var arg_1 = vec3<i32>(1i);
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec3<i32> = insertBits(arg_0, arg_1, arg_2, arg_3);
  return res;
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var tint_symbol : VertexOutput;
  tint_symbol.pos = vec4<f32>();
  tint_symbol.prevent_dce = insertBits_428b0b();
  return tint_symbol;
}

Failed to generate: :21:21 error: glsl.bitfieldInsert: no matching call to 'glsl.bitfieldInsert(vec3<i32>, vec3<i32>, i32, i32)'

1 candidate function:
 • 'glsl.bitfieldInsert(base: T  ✗ , insert: T  ✗ , offset: i32  ✓ , bits: i32  ✓ ) -> T' where:
      ✗  'T' is 'i32' or 'u32'

    %15:vec3<i32> = glsl.bitfieldInsert %6, %7, %13, %14
                    ^^^^^^^^^^^^^^^^^^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:vec3<i32> @offset(16), @location(0), @interpolate(flat)
}

%insertBits_428b0b = func():vec3<i32> {
  $B1: {
    %arg_0:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(1i)
    %arg_1:ptr<function, vec3<i32>, read_write> = var, vec3<i32>(1i)
    %arg_2:ptr<function, u32, read_write> = var, 1u
    %arg_3:ptr<function, u32, read_write> = var, 1u
    %6:vec3<i32> = load %arg_0
    %7:vec3<i32> = load %arg_1
    %8:u32 = load %arg_2
    %9:u32 = load %arg_3
    %10:u32 = min %8, 32u
    %11:u32 = sub 32u, %10
    %12:u32 = min %9, %11
    %13:i32 = convert %10
    %14:i32 = convert %12
    %15:vec3<i32> = glsl.bitfieldInsert %6, %7, %13, %14
    %res:ptr<function, vec3<i32>, read_write> = var, %15
    %17:vec3<i32> = load %res
    ret %17
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B2: {
    %tint_symbol:ptr<function, VertexOutput, read_write> = var
    %20:ptr<function, vec4<f32>, read_write> = access %tint_symbol, 0u
    store %20, vec4<f32>(0.0f)
    %21:ptr<function, vec3<i32>, read_write> = access %tint_symbol, 1u
    %22:vec3<i32> = call %insertBits_428b0b
    store %21, %22
    %23:VertexOutput = load %tint_symbol
    ret %23
  }
}


tint executable returned error: exit status 1
