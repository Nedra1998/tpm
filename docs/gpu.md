# GPU

## OpenCL

When using OpenCL, the internal GPU load balancer will construct a
`cl::Context`, and `cl::Image2D` for each OpenCL platform available on the
system. The kernel code is compiled for each platform, then the load balancer
will handle distributing the work amongst the available devices and platforms.

The method that the tasks are distributed is according the maximum clock speed
on each device. The slowest device will be allocated a minimum of one tile for
that device to render, and each other device is allocated a proportial number of
tiles accordingly to its perportial performance (ie a device with twice the
speed will be allocated twice the number of tiles).
