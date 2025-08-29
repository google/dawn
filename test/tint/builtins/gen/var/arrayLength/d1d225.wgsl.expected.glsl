SKIP: FAILED

Failed to generate: :18:17 error: length: no matching call to 'length(resource_binding)'

1 candidate function:
 • 'length(ptr<storage, array<T>, A>  ✗ ) -> i32'

    %5:i32 = %4.length
                ^^^^^^

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:u32 @offset(16), @location(0), @interpolate(flat)
}

prevent_dce_block = struct @align(4), @block {
  inner:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, prevent_dce_block, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, resource_binding, read> = var undef @binding_point(1, 0)
}

%arrayLength_d1d225 = func():u32 {
  $B2: {
    %4:resource_binding = load %arg_0
    %5:i32 = %4.length
    %6:u32 = convert %5
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %10:u32 = call %arrayLength_d1d225
    %11:ptr<storage, u32, read_write> = access %1, 0u
    store %11, %10
    ret
  }
}

Failed to generate: :18:17 error: length: no matching call to 'length(resource_binding)'

1 candidate function:
 • 'length(ptr<storage, array<T>, A>  ✗ ) -> i32'

    %5:i32 = %4.length
                ^^^^^^

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:u32 @offset(16), @location(0), @interpolate(flat)
}

prevent_dce_block = struct @align(4), @block {
  inner:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, prevent_dce_block, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, resource_binding, read> = var undef @binding_point(1, 0)
}

%arrayLength_d1d225 = func():u32 {
  $B2: {
    %4:resource_binding = load %arg_0
    %5:i32 = %4.length
    %6:u32 = convert %5
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %10:u32 = call %arrayLength_d1d225
    %11:ptr<storage, u32, read_write> = access %1, 0u
    store %11, %10
    ret
  }
}

Failed to generate: :13:17 error: length: no matching call to 'length(resource_binding)'

1 candidate function:
 • 'length(ptr<storage, array<T>, A>  ✗ ) -> i32'

    %4:i32 = %3.length
                ^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:u32 @offset(16), @location(0), @interpolate(flat)
}

$B1: {  # root
  %arg_0:ptr<handle, resource_binding, read> = var undef @binding_point(1, 0)
}

%arrayLength_d1d225 = func():u32 {
  $B2: {
    %3:resource_binding = load %arg_0
    %4:i32 = %3.length
    %5:u32 = convert %4
    %res:ptr<function, u32, read_write> = var %5
    %7:u32 = load %res
    ret %7
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %10:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %10, vec4<f32>(0.0f)
    %11:ptr<function, u32, read_write> = access %out, 1u
    %12:u32 = call %arrayLength_d1d225
    store %11, %12
    %13:VertexOutput = load %out
    ret %13
  }
}

//
// fragment_main
//
//
// compute_main
//
//
// vertex_main
//

tint executable returned error: exit status 1
