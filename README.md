# Cotton Color

## 颜色检测

### 艳丽色彩检测

纯色彩检测就可以。饱和度检测。

思路：rgb -> HSV -> s -> threshold -> 杂质 

### 黑色检测/滴灌带检测

L a* b* 色彩空间检测，纯黑色就是杂质，但是容易有噪声。

思路：rgb -> La*b* -> threshold -> 黑色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

### 暗黄色检测/油棉

L a* b* 色彩空间检测，检测暗黄色。

思路：rgb -> La*b* -> threshold -> 暗黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

### 带土地膜检测

L a* b* 色彩空间检测，检测明黄色、白色。

思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质


## 深度学习检测

需求：模板匹配缺少对于纹理的判断，所以要加上深度学习对于各个杂质进行确认。

### 方案1：端到端式的方案

传统思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

深度学习思路：rgb -> 可疑色彩图像增强 -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质
						              |
						               -> YOLO -> 杂质


### 方案2：验证形式的方案

传统思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质

融合深度学习思路：rgb -> La*b* -> threshold -> 白色、明黄色 -> 模板匹配 -> 物体的大小 -> 阈值判断 -> 杂质
															|									  |
									激进方案    			 -> 深度学习 - > 区块判别 -> 杂质     |
																					      |
									保守方案											   -> 深度学习确认 -> 杂质
															
## 项目配置过程

### 1. opencv依赖

1. 包含目录 (Additional Include Directories)
这个属性告诉编译器在哪里查找 OpenCV 的头文件。
需要添加 OpenCV 的 include 文件夹路径。
步骤：

	右键点击你的项目 -> Properties（属性）。在左侧菜单中，选择 Configuration Properties -> C/C++ -> General。
找到 Additional Include Directories。
在该项中添加 OpenCV 的头文件路径，例如：

```makefile
C:\opencv\build\include
```

	这里的 `C:\opencv\build\include`是 OpenCV 的包含文件夹，假设你把 OpenCV 安装在 C:\opencv 目录下。

2. 库目录 (Additional Library Directories)
	这个属性告诉链接器在哪里查找 OpenCV 的库文件。
	需要添加 OpenCV 的 x64/vc15/lib 或 x86/vc15/lib 目录（根据你的架构选择）。
	步骤：
					
	在项目属性页中，选择 Configuration Properties -> Linker -> General。
找到 Additional Library Directories。
添加 OpenCV 的库文件路径。例如：
```vbnet
复制代码
C:\opencv\build\x64\vc15\lib
```
这里 x64 表示 64 位版本，如果你使用的是 32 位版本，则路径应该是 x86。
	
3. 附加依赖项 (Additional Dependencies)
这个属性告诉链接器需要链接哪些 OpenCV 库文件。需要在此添加 .lib 文件。
通常，你可以链接 opencv_world4xx.lib（例如 opencv_world453.lib，具体版本根据你安装的 OpenCV 版本而定）。
步骤：

在项目属性页中，选择 Configuration Properties -> Linker -> Input。
找到 Additional Dependencies，并添加需要的 .lib 文件。比如：
vbnet
复制代码
opencv_world453.lib
根据你的 OpenCV 版本，.lib 文件名会有所不同，比如 opencv_world400.lib，opencv_world430.lib 等等。
如果你不确定需要哪些 .lib 文件，可以参考 OpenCV 安装目录下 lib 文件夹中的所有 .lib 文件。

4. 运行时 DLL 文件路径 (Path for Runtime DLLs)
你还需要确保在运行时，程序能够找到 OpenCV 的动态链接库（DLL 文件）。这通常通过将 OpenCV 的 bin 文件夹路径添加到系统的 Path 环境变量中来实现。
bin 文件夹通常位于：
makefile
复制代码
C:\opencv\build\x64\vc15\bin
步骤：

右键点击桌面上的 计算机 或 此电脑。
选择 属性 -> 高级系统设置 -> 环境变量。
在 系统变量 中，选择 Path，然后点击 编辑。
在 编辑环境变量 窗口中，点击 新建，并添加 OpenCV bin 文件夹路径：
makefile
复制代码
C:\opencv\build\x64\vc15\bin

### 2. windows 功能依赖

在 Visual Studio 中的配置：
右击项目 -> "属性"。
在 "链接器" -> "输入" -> "附加依赖项" 中，添加 Comdlg32.lib。
确保 Windows.h 和 CommDlg32.h 被正确包含。
