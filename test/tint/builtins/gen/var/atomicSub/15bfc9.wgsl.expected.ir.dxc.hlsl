SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn atomicSub_15bfc9() -> u32 {
  var arg_1 = 1u;
  var res : u32 = atomicSub(&(sb_rw.arg_0), arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atomicSub_15bfc9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicSub_15bfc9();
}

Failed to generate: :15:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %7:u32 = negation %5
    ^^^^^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
SB_RW = struct @align(4) {
  arg_0:atomic<u32> @offset(0)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
  %sb_rw:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 1)
}

%atomicSub_15bfc9 = func():u32 {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var, 1u
    %5:u32 = load %arg_1
    %6:ptr<function, u32, read_write> = var, 0u
    %7:u32 = negation %5
    %8:u32 = convert 0u
    %9:void = %sb_rw.InterlockedAdd %8, %7, %6
    %10:u32 = load %6
    %res:ptr<function, u32, read_write> = var, %10
    %12:u32 = load %res
    ret %12
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %14:u32 = call %atomicSub_15bfc9
    %15:u32 = bitcast %14
    %16:void = %prevent_dce.Store 0u, %15
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %18:u32 = call %atomicSub_15bfc9
    %19:u32 = bitcast %18
    %20:void = %prevent_dce.Store 0u, %19
    ret
  }
}


tint executable returned error: exit status 1
