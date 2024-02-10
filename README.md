# vkperf

Vulkan Performance Measurement Tool

vkperf tests various performance characteristics of Vulkan devices.
It is aimed at developer needs. It provides developers with performance characteristics of their Vulkan devices,
allowing them to make better decisions during the design of their rendering algorithms and the design of Vulkan-related code in general.

The tests cover six main areas:
* Triangle throughput
* Vertex and geometry shader throughput
* Attributes and buffers overhead
* Transformations
* Fragment throughput
* Transfer throughput

For more information, see [sample vkperf output](SampleOutput.md), [Tests overview section](#tests-overview)
or [Detailed test description](ListOfTests.md).


## Usage

```
vkperf [deviceNameFilter] [deviceIndex] [options]
   --long - perform long test; testing time is extended to 20 seconds
            from the default of 2 seconds
   --minimal - perform minimal test; for debugging purposes,
               number of triangles used for testing is lagerly
               reduced, making measurements possibly very imprecise
   --sparse-none - no sparse mode is used during the main test;
                   this is the default
   --sparse-binding - sparse binding mode is used during the main test
   --sparse-residency - sparse residency mode is used during the main test
   --sparse-residency-aliased - sparse residency aliased mode is used
                                during the main test
   --debug - print additional debug info
   --help or -h - prints the usage information
```


## Tests overview

* Triangle throughput
  * Triangle list
  * Triangle strips
  * Indexed rendering
  * Primitive restart
  * Instancing performance
  * vkCmdDraw() and vkCmdDrawIndexed() throughput
  * VkDrawIndirectCommand and VkDrawIndexedIndirectCommand performance
* Vertex and geometry shader throughput
  * Minimal vertex shader
  * Simple vertex shader utilizing VertexIndex and InstanceIndex
  * Geometry shader producing no output
  * Geometry shader producing single triangle
  * Geometry shader producing two triangles
* Attributes and buffers
  * Attribute input overhead for one to four attributes
  * Buffer read overhead for one to four attributes
  * Overhead for vec4 and vec3 buffer reads
  * Interleaved attributes and interleaved buffers performance
  * Packed buffers performance (using half-floats and other tricks to store position+normal+color+texCoord in 2x vec4)
* Transformations
  * Transformation matrices overhead for one to five matrices (model, view, projection, normal-model, normal-view)
  * Per-triangle matrices and scene uniform matrices
  * Phong light model overhead
  * Quaternions performance
  * Double precision transformation performance
* Fragment throughput
  * Fragment throughput of full-framebuffer quad
  * Interpolators overhead
  * Textured phong light model performance
  * Simplified phong light model performance
* Transfer throughput
  * Various block sizes transfer performance from host to device memory


## License

Public domain


## Author

PC John (Jan Pečiva, peciva _at fit.vut.cz, https://github.com/pc-john, https://www.fit.vut.cz/~peciva)
