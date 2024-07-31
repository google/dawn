SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat4x3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat4x3<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %26 declared here
    %26:u32 = mul %50, 2u
    ^^^^^^^

:45:24 error: binary: %26 is not in scope
    %32:u32 = add %31, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %26 declared here
    %26:u32 = mul %50, 2u
    ^^^^^^^

:50:24 error: binary: %26 is not in scope
    %38:u32 = add %37, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:62:5 note: %26 declared here
    %26:u32 = mul %50, 2u
    ^^^^^^^

:62:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %26:u32 = mul %50, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat4x3<f16> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:u32 = add %10, %13
    %24:u32 = add %23, %16
    %25:u32 = add %24, %26
    %27:array<Inner, 4> = call %28, %25
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %26
    %33:Inner = call %34, %32
    %l_a_i_a_i:Inner = let %33
    %36:u32 = add %10, %13
    %37:u32 = add %36, %16
    %38:u32 = add %37, %26
    %39:mat4x3<f16> = call %40, %38
    %l_a_i_a_i_m:mat4x3<f16> = let %39
    %42:u32 = add %10, %13
    %43:u32 = add %42, %16
    %44:u32 = div %43, 16u
    %45:ptr<uniform, vec4<u32>, read> = access %a, %44
    %46:vec4<u32> = load %45
    %47:vec4<f16> = bitcast %46
    %48:vec3<f16> = swizzle %47, xyz
    %l_a_i_a_i_m_i:vec3<f16> = let %48
    %50:i32 = call %i
    %26:u32 = mul %50, 2u
    %51:u32 = add %10, %13
    %52:u32 = add %51, %16
    %53:u32 = add %52, %26
    %54:u32 = div %53, 16u
    %55:ptr<uniform, vec4<u32>, read> = access %a, %54
    %56:u32 = mod %53, 16u
    %57:u32 = div %56, 4u
    %58:u32 = load_vector_element %55, %57
    %59:u32 = mod %53, 4u
    %60:bool = eq %59, 0u
    %61:u32 = hlsl.ternary 16u, 0u, %60
    %62:u32 = shr %58, %61
    %63:f32 = hlsl.f16tof32 %62
    %64:f16 = convert %63
    %l_a_i_a_i_m_i_i:f16 = let %64
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %67:array<Inner, 4> = call %28, %start_byte_offset
    %68:Outer = construct %67
    ret %68
  }
}
%28 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x3<f16>(vec3<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %72:bool = gte %idx, 4u
        if %72 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %73:u32 = mul %idx, 64u
        %74:u32 = add %start_byte_offset_1, %73
        %75:ptr<function, Inner, read_write> = access %a_1, %idx
        %76:Inner = call %34, %74
        store %75, %76
        continue  # -> $B8
      }
      $B8: {  # continuing
        %77:u32 = add %idx, 1u
        next_iteration %77  # -> $B7
      }
    }
    %78:array<Inner, 4> = load %a_1
    ret %78
  }
}
%34 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %80:mat4x3<f16> = call %40, %start_byte_offset_2
    %81:Inner = construct %80
    ret %81
  }
}
%40 = func(%start_byte_offset_3:u32):mat4x3<f16> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %83:u32 = div %start_byte_offset_3, 16u
    %84:ptr<uniform, vec4<u32>, read> = access %a, %83
    %85:vec4<u32> = load %84
    %86:vec4<f16> = bitcast %85
    %87:vec3<f16> = swizzle %86, xyz
    %88:u32 = add 8u, %start_byte_offset_3
    %89:u32 = div %88, 16u
    %90:ptr<uniform, vec4<u32>, read> = access %a, %89
    %91:vec4<u32> = load %90
    %92:vec4<f16> = bitcast %91
    %93:vec3<f16> = swizzle %92, xyz
    %94:u32 = add 16u, %start_byte_offset_3
    %95:u32 = div %94, 16u
    %96:ptr<uniform, vec4<u32>, read> = access %a, %95
    %97:vec4<u32> = load %96
    %98:vec4<f16> = bitcast %97
    %99:vec3<f16> = swizzle %98, xyz
    %100:u32 = add 24u, %start_byte_offset_3
    %101:u32 = div %100, 16u
    %102:ptr<uniform, vec4<u32>, read> = access %a, %101
    %103:vec4<u32> = load %102
    %104:vec4<f16> = bitcast %103
    %105:vec3<f16> = swizzle %104, xyz
    %106:mat4x3<f16> = construct %87, %93, %99, %105
    ret %106
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x3<f16>(vec3<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %110:bool = gte %idx_1, 4u
        if %110 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %111:u32 = mul %idx_1, 256u
        %112:u32 = add %start_byte_offset_4, %111
        %113:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %114:Outer = call %21, %112
        store %113, %114
        continue  # -> $B15
      }
      $B15: {  # continuing
        %115:u32 = add %idx_1, 1u
        next_iteration %115  # -> $B14
      }
    }
    %116:array<Outer, 4> = load %a_2
    ret %116
  }
}

