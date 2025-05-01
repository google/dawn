# Copyright 2024 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

function(bundle_libraries output_target library_type)

  # 1. Validate the library_type argument
  string(TOUPPER ${library_type} upper_library_type)
  if(NOT (upper_library_type STREQUAL "STATIC" OR upper_library_type STREQUAL "SHARED"))
    message(FATAL_ERROR "bundle_libraries: Invalid library type specified '${library_type}'. Must be STATIC or SHARED for target '${output_target}'.")
    return() # Fatal error stops execution, but return is good practice
  endif()

  # Use the validated type in upper case
  set(_target_type ${upper_library_type})

  # --- Inner helper function to get dependencies ---
  # This function recursively finds all dependencies of a given target.
  # It populates a list variable named 'all_dependencies' in the parent scope.
  # Note: This recursive approach with PARENT_SCOPE works but can be a bit
  # tricky with variable lifetimes. It assumes 'all_dependencies' is
  # initialized in the calling scope.
  function(get_dependencies input_target)
    # Resolve aliases first
    get_target_property(alias ${input_target} ALIASED_TARGET)
    if(TARGET ${alias})
      set(input_target ${alias})
    endif()

    # Avoid processing the same target multiple times (circular dependency check)
    # Checks if the current target is already in the list being built in the parent scope.
    # Note: all_dependencies is modified in parent scope below, so check works.
    if(${input_target} IN_LIST all_dependencies)
      #message("DEBUG: Already processed ${input_target}")
      return()
    endif()

    # Add the current target to the list in the parent scope
    # list(APPEND) modifies the variable in the current scope.
    # set(... PARENT_SCOPE) copies the current scope's variable to the parent scope.
    # This pattern, while slightly verbose, works for recursive list building across scopes.
    list(APPEND all_dependencies ${input_target})
    set(all_dependencies ${all_dependencies} PARENT_SCOPE) # Propagate change up

    # Get dependencies from LINK_LIBRARIES (Private and Public linkage)
    get_target_property(link_libraries ${input_target} LINK_LIBRARIES)
    foreach(dependency IN LISTS link_libraries)
      # Only recurse on actual targets
      if(TARGET ${dependency})
        #message("DEBUG: Recursing into LINK_LIBRARIES dependency: ${dependency}")
        get_dependencies(${dependency}) # Recursive call
      endif()
    endforeach()

    # Get dependencies from INTERFACE_LINK_LIBRARIES (Interface and Public linkage)
    get_target_property(interface_link_libraries ${input_target} INTERFACE_LINK_LIBRARIES)
    foreach(dependency IN LISTS interface_link_libraries)
      # Only recurse on actual targets
      if(TARGET ${dependency})
        get_dependencies(${dependency}) # Recursive call
      endif()
    endforeach()

    # The final 'all_dependencies' list is available in the parent scope
    # after the initial call to get_dependencies completes.

  endfunction() # End of get_dependencies helper function
  # --- End of inner helper function ---

  # Initialize the list that the recursive function will populate.
  # This list will exist in the scope of the bundle_libraries function.
  set(all_dependencies "")

  # 2. Get dependencies for all input targets provided in ARGN
  # ARGN contains all arguments *after* the named parameters (output_target, library_type)
  if(NOT ARGN)
     message(FATAL_ERROR "bundle_libraries: No input targets specified for '${output_target}'.")
     return()
  endif()

  foreach(input_target IN LISTS ARGN)
    if(TARGET ${input_target})
      #message("DEBUG: Starting dependency analysis for input target: ${input_target}")
      get_dependencies(${input_target})
    else()
      message(WARNING "bundle_libraries: Input target '${input_target}' is not a valid target. Skipping.")
    endif()
  endforeach()

  #message("DEBUG: Collected dependencies: ${all_dependencies}")

  # 3. Collect $<TARGET_OBJECTS:...> from STATIC and OBJECT library dependencies
  set(all_objects "")
  foreach(dependency IN LISTS all_dependencies)
    get_target_property(type ${dependency} TYPE)
    # message("DEBUG: Checking dependency ${dependency} with type ${type}")

    # We only want object files from static or object libraries.
    # This correctly excludes shared libraries, modules, executables, interfaces, etc.
    if(${type} STREQUAL "STATIC_LIBRARY")
      list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
    elseif(${type} STREQUAL "OBJECT_LIBRARY")
      list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
    endif()
  endforeach()

  # message("DEBUG: Collected objects: ${all_objects}")

  # Check if any object files were found
  if(NOT all_objects)
    message(WARNING "bundle_libraries: No object files found from the dependencies of ${ARGN}. Creating empty library '${output_target}' as ${_target_type}.")
  endif()

  # 4. Create the output library using the validated type and collected objects
  # If all_objects is empty, add_library will still create an empty library of the specified type.
  add_library(${output_target} ${_target_type} ${all_objects})

  # 5. Add dependencies to ensure input targets are built before the bundled library.
  # This handles the build order correctly.
  add_dependencies(${output_target} ${ARGN})

endfunction()
