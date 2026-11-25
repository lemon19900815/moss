# `InstallShield`(简称IS)

## 1. 基础使用方法
  
  - https://blog.csdn.net/plfl520/article/details/4358968
  - https://blog.csdn.net/dy_paradise/article/details/8254200
  
- 常见问题
  - IS在使用过程中经常卡死情况：[参考该链接进行修改](https://blog.csdn.net/weixin_43976890/article/details/108580457)
  - IS打包x64平台版本出错：”ISDEV : error -5008: Intel64 or AMD64 must be specified in the template of the Summary Stream“，通过修改"Installation Information->General Information->Summary Information Stream->Template Summary"的字典值“Intel;1033->x64;1033”
  
## 2. 开机自启动设置

  - 安装程序之后，在运行(Win+R)内输入`shell:startup`进入目录，在当前目录下创建快捷方式关联程序；

  - 在IS内部通过注册表的方式

    - 注册表内添加需要运行的程序：`HKEY_CURRENT_USER/SOFTWARE/Microsoft/Windows/CurrentVersion/Run`（如：value为`D:\lemon\test.exe`，key随意）；
    - 这里需要注意的是，程序运行的当前工作路径为启动的位置（如：我在c:\system32目录启动这个程序，则当前目录为c:\system32），如果程序内部使用的相对路径，则可能由于权限等原因，不能正常启动；
    - 设置`vbs`脚本的方式避免当前路径问题（开机自启程序需要设置为start.vbs）

    ```vbscript
    ' 开机自启动软件脚本（bat脚本启动时会存在黑框，所以通过vbs脚本规避）
    
    currentpath = createobject("Scripting.FileSystemObject").GetFile(Wscript.ScriptFullName).ParentFolder.Path
    ' MsgBox "cur_dir: " & currentpath
    
    ' 切换当前工作路径到脚本路径
    set ws=WScript.CreateObject("WScript.Shell")
    ws.currentdirectory = currentpath
    
    ' 启动软件
    ws.Run ".\test.exe"
    ```

## 3. 设置Dynamic File Linking

通过文件夹或文件添加的方式添加到打包中时，有以下缺陷：
- 在指定目录下增加文件时，还需要返回来修改打包的工程文件；
- 并且以这样方式添加的是绝对路径，不是相对路径，在其他位置不可移植；

所以通过Dynamic File Linking的方式可以解决这个问题，具体处理方式见下图：
![](img/InstallShield使用说明.png)