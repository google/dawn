# NXT, standalone part

NXT is an unoriginal name for Chromium’s investigation and prototyping of a NeXT generation graphics API for the Web. This repository contains a native library implementing NXT on multiple backends, as well as some code generators used for the integration in Chromium. NXT is not an official Google product.

We focused on efforts on two axis:

- An investigation of the constraints coming from the Web and in particular portability, for which we looked at the intersection of the designs of D3D12, Metal, Vulkan, OpenGL and D3D11. See links to some of our investigations below.
- A prototype API inspired by all of D3D12, Metal and Vulkan, but none in particular. The API works on two backends: OpenGL and Metal and is usable from native code (think WebAssembly) and from Javascript inside of Chrome. Our focus was not to have a complete API but to show the breadth of potential usage.

We’re making our investigation and prototype public to provide another example for the upcoming discussion in the “GPU for the Web” W3C community group.

NXT currently has the following features:

- Command buffers, graphics and compute pipelines
- Textures, samplers, vertex / index / uniform / storage buffers.
- Descriptor sets (called bind groups) and push constants
- SPIRV for the shading language
- Validation

NXT is missing a lot of things to be usable for anything else than prototyping:

- Render-targets / render passes
- Most of the fixed function pipeline state
- Barriers / resource transitions and GPU - CPU synchronization
- Buffer mapping
- ...

We chose to use SPIRV in our prototype because it was the only language that had translators to other shading languages, thanks to SPIRV-Cross which saved us a ton of work. [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) didn’t have an HLSL backend so we didn’t attempt a D3D12 backend (and D3D11 was too limiting for our prototype API). Only the Metal and the OpenGL backends are functional at this time. The OpenGL backend let us integrate in Chromium very easily.

## Links

An [overview document](https://docs.google.com/document/d/1-lAvR9GXaNJiqUIpm3N2XuGUWv_JrkpGizDN0bNq7wY) of the goals and design of NXT.

Some of the investigations we made on the design of potential backend APIs:

- [Binding model investigation](https://drive.google.com/open?id=1_xeTnk6DlN7YmePQQAlnHndA043rgwBzUYtatk5y7kQ)
- [Data uploads investigation](https://drive.google.com/open?id=1Mi9l14zG8HzJ5Z6107SdPhON0mq4d-3SUI8iS631nek)
- [Resource creation investigation](https://drive.google.com/open?id=1hK1SkTFkXJXPjyla0EEl1fOIwJSc6T41AV2mGiovyFU)
- [Vertex setup investigation](https://drive.google.com/open?id=1SIUpdg-6Xm5FFF1ktdBfnR5oRKjyPAfXir7Drui4cYM)

[Another presentation](https://drive.google.com/open?id=1mLQEM__twfivV7nJLDBIomS9pegOYkJQWyM6lTse4PQ) about our work with more details on the architecture of the prototype, and a [video](https://youtu.be/ThlZ5K4hJvo) of the demo we showed.

TODO: add a link to the NXT-chromium repo once it is uploaded.

## Key elements of the prototype’s architecture

### Builder pattern for object creation

In NXT, object creation is done through builder objects that gather initialization parameters with a fluent interface and return the initialized object when GetResult() is called. 

In addition to the improved type-safety and subjective prettiness compared to giant constructors, this style enables additional optimizations. For example this removes the need for any check for an object being built and allows backend to forget parameters it might not care about.

Here’s an example of buffer creation:

```cpp
nxt::Buffer buffer = device.CreateBufferBuilder()
    .SetUsage(nxt::BufferUsageBit::Uniform | nxt::BufferUsageBit::Mapped)
    .SetSize(42)
    .GetResult();
```

### The wire client-server and error handling
What we call the wire is an API command buffer for NXT. The name was chosen to avoid conflict with the “command buffer” concept in graphics APIs.

Originally OpenGL was designed as a client-server architecture with asynchronous errors and objects that could be used by the client before they were created by server. Over time more client state-tracking was added but the core asynchronous structure remained. This enabled OpenGL ES 2 / WebGL to be implemented in Chromium in which the web page and the GPU driver live in different processes. In addition to security this separation helps with performance in CPU-bound WebGL apps.

For this reason we built NXT as a network-transparent API so that it could integrate nicely in the Chromium architecture, and we believe any next-generation Web API would have to be network-transparent too.

In NXT, as in OpenGL, API objects can be used immediately after they have been created on the client, even if the server hasn’t seen the creation command yet. If object creation succeeds, everything happens transparently otherwise the object is tagged as being an error. NXT calls with error-tagged objects use the following rules:

- Functions result in a noop.
- Functions returning an object return an error value.
- Builder methods mark the builder as an error value.

The idea is that a whole bunch of object creation can be done when the application loads, then all the objects checked once for any error. The concept presented above is similar to [promise pipelining](http://www.erights.org/elib/distrib/pipeline.html) and to the [Maybe monad](https://en.wikipedia.org/wiki/Monad_(functional_programming)#The_Maybe_monad).

Currently the wire only has client to server communication and there is no way to know the error status of objects or read API data like the content of buffers. In our prototype the wire is responsible for object lifetime validation.

### Code generation

Our prototype heavily relies on code generation. It greatly improved iteration time on the API as the generators kept the Javascript bindings, IDL files, wire, C++ bindings and friends up to date. But it reduced flexibility in the API shape as adding as changing the shape required modifying all generators in non-trivial ways.

For example, NXT can only return objects which prevents mapping buffers or even reading back single pixel values. There is currently no way to know the error status on the client side. These improvements, and more, are planned, and contributions are welcome.

Other generators include:

- A C-header with the definition of nxtProcTable that is the “real” underlying NXT API exposed by the backends.
- Glue code generating nxtProcTable for a backend with simple validation included (enum value checks etc.)
- A mock API for testing

## Structure of the code

Here are the main files and directories:

```
/next.json - the JSON file describing the API that is used by the code generators
/examples - example code that was also used for end2end testing (it is not possible to do automated testing without being able to read back data)
/generator - The code generator and its templates
/generator/templates - The code generator templates
/generator/templates/blink - Templates used in the integration with Chromium
/src - Non-generator code for the ANGLE-like library
/src/backend
/src/backend/common - Handles all the state tracking and validation
/src/backend/metal - the Metal backend
/src/backend/opengl - the OpenGL backend
/src/wire - Glue code and interfaces for the wire
/third_party - external dependencies
```

## Getting and building the code

NXT standalone is a CMake project with git submodules. To download and build it, do the following:

```sh
git clone --recursive <insert github git repo url here>
cd <directory name>
mkdir build && cd build
cmake ..
make
# Run executables in examples/, --help will provide the options to choose the backend (compute only works on Metal on OSX) and the command buffer.
```

It is currently known to compile on Linux and OSX, and has some warnings on Windows when using MSVC (it doesn’t handle code reachability in enum class switches correctly).
