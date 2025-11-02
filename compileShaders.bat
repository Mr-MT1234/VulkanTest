@echo off
for %%f in (VulkanTest1/res/shaders/*.vert VulkanTest1/res/shaders/*.frag) do (echo ------Compiling %%f------ & glslc.exe VulkanTest1/res/shaders/%%f -o VulkanTest1/res/shaders/spir-v/%%f.spv)

pause