SKIP: FAILED


struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<u32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : u32,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicSub_15bfc9() {
  var arg_1 = 0u;
  var res = 0u;
  arg_1 = 1u;
  let x_18 = arg_1;
  let x_13 = atomicSub(&(sb_rw.arg_0), x_18);
  res = x_13;
  return;
}

fn fragment_main_1() {
  atomicSub_15bfc9();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicSub_15bfc9();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}

Failed to generate: :17:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %8:u32 = negation %x_18
    ^^^^^^^^^^^^^^^^^^^^^^^

:10:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
SB_RW_atomic = struct @align(4) {
  arg_0:atomic<u32> @offset(0)
}

$B1: {  # root
  %sb_rw:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
}

%atomicSub_15bfc9 = func():void {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var, 0u
    %res:ptr<function, u32, read_write> = var, 0u
    store %arg_1, 1u
    %5:u32 = load %arg_1
    %x_18:u32 = let %5
    %7:ptr<function, u32, read_write> = var, 0u
    %8:u32 = negation %x_18
    %9:u32 = convert 0u
    %10:void = %sb_rw.InterlockedAdd %9, %8, %7
    %11:u32 = load %7
    %x_13:u32 = let %11
    store %res, %x_13
    ret
  }
}
%fragment_main_1 = func():void {
  $B3: {
    %14:void = call %atomicSub_15bfc9
    ret
  }
}
%fragment_main = @fragment func():void {
  $B4: {
    %16:void = call %fragment_main_1
    ret
  }
}
%compute_main_1 = func():void {
  $B5: {
    %18:void = call %atomicSub_15bfc9
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B6: {
    %20:void = call %compute_main_1
    ret
  }
}


tint executable returned error: exit status 1
