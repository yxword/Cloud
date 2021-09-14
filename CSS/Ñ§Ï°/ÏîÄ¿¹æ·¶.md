# GitLab使用规范

## 公司统一规范

```
https://yfgitlab.dahuatech.com/devopsguide/docs/raw/master/gitlab%E4%BD%BF%E7%94%A8%E8%A7%84%E8%8C%83/Gitlab%E4%BD%BF%E7%94%A8%E8%A7%84%E8%8C%83%EF%BC%88%E4%B8%89%EF%BC%89.PNG
```

## log 规范

```
格式：
type:一句话描述本次修改内容  
详细描述本次修改内容，可以多行，也可以省略
redmine #redmine单号

type类型：feat新功能、fix（修补bug）、docs（文档）、style（改格式）、refactor（重构）、test（加测试）、chore（构建过程或辅助工具变动）

具体示例：
fix:修复并发读写流程问题       
1、修复读写时元数据数据没有锁保护
2、修复读写时程序core问题
redmine #64270
```

## 使用规范

* 要拉分支修改，之后再提交请求合并到主干
* ==尽量使用windows下git比较工具==，防止提交异常代码
* 提交前需确认本地分支名和版本，防止本地代码太老
* ==冲突修改尽量在windows下使用IDE修改，防止解决冲突出现错误==
* 本地分支和远端保持同名
* 自己创建的分支，用完后提交代码合并后，及时删除



# Redmine规范

参考模板

```
http://10.31.17.179/redmine/projects/magiccubg/files
```

# Paas  Wiki和Wetrack



# 版本转测规范

```
https://yfgitlab.dahuatech.com/BigData/CloudStorageDepartment/teaminfomation/blob/master/06.%E9%A1%B9%E7%9B%AE%E7%AE%A1%E7%90%86/%E9%A1%B9%E7%9B%AE%E8%A7%84%E8%8C%83/%E7%89%88%E6%9C%AC%E8%BD%AC%E6%B5%8B%E8%A7%84%E8%8C%83.md
```

# 代码评审规范

# 研发常用网站操作

* ==容器云==
* wetest
* weci
  。。。
* 