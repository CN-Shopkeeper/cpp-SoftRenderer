# cpp-SoftRenderer

使用纯 cpp 编写的 3D 软渲染器。

## 鸣谢

感谢`单身剑法传人`（[B 站](https://space.bilibili.com/256768793/) [GitHub](https://github.com/VisualGMQ)），没有他的帮助我根本完成不了这个项目。

## 参考项目

- [rs-cpurenderer](https://github.com/VisualGMQ/rs-cpurenderer)
- [RenderHelp](https://github.com/skywind3000/RenderHelp)
- [SoftRenderer](https://github.com/VisualGMQ/SoftRenderer)

## 运行方式

1. 在命令行下(MinGW)：

   ```
   cmake -G "MinGW Makefiles" -S . -B cmake-build -DSDL2_ROOT=xxx -DSDL2_IMAGE_ROOT=xxx -DSDL2_TTF_ROOT=xxx

   cmake --build cmake-build
   ```

2. vscode + cmake tools

   在.vscode/settings.json 中添加

   ```
   "cmake.configureArgs": [
       "-DSDL2_ROOT=xxx",
       "-DSDL2_IMAGE_ROOT=xxx",
       "-DSDL2_TTF_ROOT=xxx"
   ],
   ```

## 操作提示

- w/a/s/d: (摄像机)前进/左移/后退/右移
- q/e: (摄像机)上升/下降
- t: 切换视图模式

- 模型切换:
  - 1 -> Red Bird
  - 2 -> Son Goku
  - 3 -> White Cube
  - 4 -> Reckless Shopkeeper!

通过 `CMakeLists.txt` 中的 `add_compile_definitions()`可以更改渲染方式(CPU 或 GPU)

```cmake
# gpu
add_compile_definitions(GPU_FEATURE_ENABLED)

# cpu
# add_compile_definitions(CPU_FEATURE_ENABLED)
```
