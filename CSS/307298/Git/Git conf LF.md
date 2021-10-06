## 问题描述

* Tortoise Git Clone下来的代码，默认换行为Windows（CR LF）
* 导致代码文件传到Linux下运行，报格式错误

## 解决方法

* 原因：

```
Git 会自动识别系统;若为Windows系统
Clone时会自动将换行转为Windows风格(CRLF);
Push时会自动将换行转为Unix风格(LF).
```

方法一：TortoiseGit

```
TortoiseGit -> Settings -> Git -> Global
AutoCrLf 设置成 false
（SafeCrLf 设置成 warn）
```

方法二：Git Bash

```bash
git config --global core.autocrlf false
#git config --global core.safecrlf warn
```





