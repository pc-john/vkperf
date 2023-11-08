# vkperf tests

Vulkan Performance Measurement Tool perform various tests to measure performance characteristics of particular Vulkan device.
The tests are described in detail in this file.

The list of tests follows:
1. [VS max throughput](#vs-max-throughput)
2. [VS VertexIndex and InstanceIndex forming output](#vs-vertexindex-and-instanceindex-forming-output)
3. [GS max throughput when no output is produced](#gs-max-throughput-when-no-output-is-produced)
4. [GS max throughput when single constant triangle is produced](#gs-max-throughput-when-single-constant-triangle-is-produced)
5. [GS max throughput when two constant triangles are produced](#gs-max-throughput-when-two-constant-triangles-are-produced)
6. [Instancing throughput of vkCmdDraw()](#instancing-throughput-of-vkcmddraw)
7. [Instancing throughput of vkCmdDrawIndirect()](#instancing-throughput-of-vkcmddrawindirect)
8. [Draw command throughput](#draw-command-throughput)
9. [Draw command throughput with vec4 attribute](#draw-command-throughput-with-vec4-attribute)
10. [Indirect command processing throughput](#indirect-command-processing-throughput)
11. [Indirect command processing throughput with vec4 attribute](#indirect-command-processing-throughput-with-vec4-attribute)


## VS max throughput

Vertex shader maximum throughput test measures number of triangles per second that particular Vulkan device can render.
To get vertex throughput, multiply the result by 3.

The test uses simple vertex shader with constant output. Thus, zero size triangle is produced.
The shader's main() code is as follows:
```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```
Rendering is performed using single draw call for the whole scene:
```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

## VS VertexIndex and InstanceIndex forming output

Vertex shader maximum throughput test uses simple shader that utilizes gl_VertexIndex and gl_InstanceIndex input variables.
No attributes, buffers, descriptor sets, push constants or specialization constants are used:
```c++
void main() {
	gl_Position = vec4(0, 0, float(gl_VertexIndex + gl_InstanceIndex) * 1e-20, 1);
}
```
Rendering is performed using single draw call for the whole scene:
```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

## GS max throughput when no output is produced

Geometry shader maximum throughput test uses empty geometry shader that produces no output:
```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
}
```
It is fed by empty vertex shader:
```c++
void main() {
}
```
Rendering is performed using single draw call for the whole scene:
```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

## GS max throughput when single constant triangle is produced

Geometry shader maximum throughput test uses the following geometry shader to produce single constant triangle:
```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
	gl_Position=vec4(0,0,0.5,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.6,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.4,1);
	EmitVertex();
}
```
It is fed by empty vertex shader:
```c++
void main() {
}
```
Rendering is performed using single draw call for the whole scene:
```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

## GS max throughput when two constant triangles are produced

Geometry shader maximum throughput test uses the following geometry shader to produce two constant triangles:
```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=6) out;
void main() {
	gl_Position=vec4(0,0,0.5,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.6,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.4,1);
	EmitVertex();
	EndPrimitive();
	gl_Position=vec4(0,0,0.7,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.8,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.9,1);
	EmitVertex();
}
```
The geometry shader is fed by empty vertex shader:
```c++
void main() {
}
```
Rendering is performed using single draw call for the whole scene:
```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

## Instancing throughput of vkCmdDraw()

Instancing throughput test measures instancing performance, e.g. number of instances per second
that particular Vulkan device can render.

The test uses single triangle instanced very many times. Thus, the measured number
of rendered triangles is equal to the number of rendered instances. 

The whole scene is rendered using single vkCmdDraw() call:
```c++
vkCmdDraw(
	commandBuffer,
	3,  // vertexCount
	numberOfTriangles,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);  // vertexCount, instanceCount, firstVertex, firstInstance
```
The vertex shader in outputs constant coordinates, thus producing zero-sized triangles:
```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

## Instancing throughput of vkCmdDrawIndirect()

Instancing throughput test of vkCmdDrawIndirect() uses single triangle instanced
many times in single VkDrawIndirectCommand struct with the following content:
```c++
indirectBufferPtr->vertexCount = 3;
indirectBufferPtr->instanceCount = numberOfTriangles;
indirectBufferPtr->firstVertex = 0;
indirectBufferPtr->firstInstance = 0;
```
The VkDrawIndirectCommand structure is processed by vkCmdDrawIndirect() call:
```c++
vkCmdDrawIndirect(
	indirectBuffer,  // buffer
	0,  // offset
	1,  // drawCount
	sizeof(VkDrawIndirectCommand)  // stride
);
```

The vertex shader in use outputs constant coordinates, thus producing zero-sized triangles:
```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

## Draw command throughput

Draw command throughput test measures performance of vkCmdDraw().
Each draw call renders single triangle. Thus, triangle rendering performance is equal
to the draw call performance in this test.

Draw code is equal to the following one:
```c++
for(uint32_t i=0; i<numberOfTriangles; i++)
	vkCmdDraw(
		commandBuffer,
		3,  // vertexCount
		1,  // instanceCount
		3*numberOfTriangles,  // firstVertex
		0   // firstInstance
	);
```
The vertex shader in use outputs constant coordinates, thus producing zero-sized triangles:
```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

## Draw command throughput with vec4 attribute

Draw command throughput test with vec4 attribute is equal to [the previous test](#draw-command-throughput)
except that it uses position attribute in vertex shader:
```c++
layout(location=0) in vec4 inPosition;
void main() {
	gl_Position = inPosition;
}
```
Vec4 coordinates are provided in a way to not produce any fragments in the rasterizer.
They are tiny triangles in between pixel sampling locations distributed roughly across the whole screen.

## Indirect command processing throughput

Indirect command processing throughput test measures the number of VkDrawIndirectCommand
processed per second. Each VkDrawIndirectCommand contains single triangle,
thus number of processed triangles is equal to the number of processed
VkDrawIndirectCommand structures. The content of VkDrawIndirectCommand is as follows:
```c++
indirectBufferPtr[i].vertexCount = 3;
indirectBufferPtr[i].instanceCount = 1;
indirectBufferPtr[i].firstVertex = i * 3;
indirectBufferPtr[i].firstInstance = 0;
```
The vertex shader in use outputs constant coordinates, thus producing zero-sized triangles:
```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```
The rendering command is as follows:
```c++
vkCmdDrawIndirect(
	commandBuffer,
	indirectBuffer.get(),  // buffer
	0,  // offset
	numTriangles,  // drawCount
	sizeof(VkDrawIndirectCommand)  // stride
);
```

## Indirect command processing throughput with vec4 attribute

Indirect command processing throughput test with vec4 attribute is equal to
[the previous test](#indirect-command-processing-throughput)
except that it uses position attribute in vertex shader:
```c++
layout(location=0) in vec4 inPosition;
void main() {
	gl_Position = inPosition;
}
```
Vec4 coordinates are provided in a way to not produce any fragments in the rasterizer.
They are tiny triangles in between pixel sampling locations distributed roughly across the whole screen.
