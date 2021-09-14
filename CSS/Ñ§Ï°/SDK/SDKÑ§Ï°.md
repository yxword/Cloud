## EFS-SDK_UserDocs

### 概述

大华云存储（EFileSystem，简称EFS）的开发指南。EFS的一个**网络客户端库**，提供接口实现**文件创建、读写、删除、获取系统信息**等功能。

### 使用说明

#### Bucket

* Bucket是一个存储空间，想在EFS上存储数据时，必须先创建Bucket，也就是所有文件都必须隶属于某一个Bucket。属性包括访问权限、配额容量、生命周期等。
* 其内部文件是扁平的，无目录概念
* 用户可以有多个Bucket，Bucket的**名字**在EFS内必须是**唯一**的，不同用户也不能存在同名Bucket
* 存储的文件无限制，但最好不要一个Bucket存储太多文件（10w+）
* 命名规范：
  长度必须在1-63字节之间；不能包含 **\ \* ? “ < > | / ‘ :** 特殊字符；
  前后不能有空格；UTF-8编码；**不区分大小写**；
  保证 Bucket 命名全局唯一的实例：**用户名+时间+随机数**

#### File

* File是EFS存储数据的基本单元，不同于一般文件系统，不同File之间不存在任何目录关系。但可以通过File的**文件名使之存在一定关系**，通过EFS-SDK提供的接口实现一般文件系统中的目录操作。
* File必须属于某一个Bucket，因此File的创建、读写、删除必须指定**全路径** `<bucketname>/<filename>`，**该路径必须是唯一的**。
  File分为BigFile和SmallFile，BigFile支持的最大大小为2TB，SmallFile支持的最大大小为100MB。
* 命名规范
  全路径小于等于255字节；不能包含字符同bucketname（除了 **/**）；其它同上；
  推荐小写方式命名；

#### N+M

* EFS使用Erase Code（纠删码）保护数据，所以在创建文件时必须指定N+M；
* 其中N即数据块，M为根据EC计算所得的校验块（冗余部分）；
* 当小于等于M个数据块损坏时，不影响数据读取。



### 基本流程

![image-20210729092727919](D:\学习\SDK学习.assets\EFS.png)

### C++SDK

* 头文件：IntTypes.h、Defs.h、EFileSystem.h、Bucket.h、File.h

* 链接库：
  Linux 64位：libEFSClient64.so，librt.so，libdl.so
  ......（windows 32/64）

* 安装
  动态库必须安装在动态库加载目录中，否则可能在运行时出现无法找到库的错误，**不能包含中文路径**。

  * 系统默认目录：Linux平台，64位 /lib64，/usr/lib64等目录
  * LD_LIBRARY_PATH环境变量指定的目录（推荐）
  * ldconfig配置文件中指定目录
  * 编译时通过n `-Wl,-rpath`指定的目录

  必须安装的动态库：

  * （见文档）Linux64位：libEFSCient64.so、libEFSClientCore64.so(仅在 5.0版本之后需要)

* ~~升级~~

* ~~兼容性~~

#### 使用介绍

##### 初始化

以下接口**不允许多线程调用**，在进程生命周期内，一般只需调用一次。命名空间是 Dahua::EFS

###### 连接EFS服务

功能：初始化CEFileSystem对象，登陆到EFS服务端

`bool CEFileSystem::initialize( const Config& cfg );`

* 接口参数

  ```C++
  //[IN] 登陆配置信息 
  typedef struct Config
  {
  	const char* address;	//EFS服务器IP地址，必传参数
  	uint32_t	port;		//EFS服务端口，必传参数
  	const char*  ilename;  //云存储用户名
  	const char* password;  //云存储用户密码
  const char* defaultRegion;  //<默认region，可不填写
  	char		reserved[116];	//保留字段
  }Config;
  
  ```

* 接口返回值

  * true
  * false 初始化失败。可以调用`getLastError() `获取错误
 （下文返回值为bool与此相同，省略）

* 示例

  ```C++
  Dahua::EFS::Config cfg;
  cfg.address = “192.168.1.1”; // EFS服务地址
  cfg.port = 38100; // EFS服务端口，固定不变
  cfg.userName = “root”;// 存储用户名
  cfg.password = “123456”; // 存储用户密码
  
  Dahua::EFS::CEFileSystem* efs = new Dahua::EFS::CEFileSystem;
  // 初始化
  if( efs->initialize( cfg ) ){
  	// 使用efs进行操作，比如创建bucket和文件
  	// 关闭后不得再使用efs
  	efs->close();
  }else{
  	std::cout << “initialize failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  delete efs;
  efs = NULL;
  ```

* 提示

  * 推荐只创建一个CEFileSystem实例
  * 因全局变量初始化顺序问题，所以不得以全局变量的方式实例化CEFileSystem对象，可以使用全局指针变量
  * 如发生网络异常，内部会自动重连。如设置了监听，那么将会回调通知使用者。

###### 高级选项配置
功能：设置高级配置信息
`bool CEFileSystem:: setOption(EFSOption key, const void* value, uint32_t len);`

* 接口参数
  key：

  ```C++
  //[IN ]系统配置选项。
  enum EFSOption
  {
  	efsConcurrent,		//并发数,取值范围[1, 20]
  	efsBufferSize,		//文件缓存大小
  	efsTimeOut,		//消息超时时间
  	efsLogOutput,		//日志文件输出路径
  	efsLogLevel,		//日志级别
  	efsRwMode,		//读写模式(0异步1同步)
  	efsSmallFileBufferSize,	//小文件缓存大小（7.2版本后弃用）
  	efsBufferMode ///<读缓存模式（7.2版本后弃用）
  };
  ```

  value：  [IN ]配置项的值。  

  len：  [IN]配置项value的大小。单位B。

* 示例

  ```C++
  Dahua::EFS::CEFileSystem* efs = new Dahua::EFS::CEFileSystem;
// 设置日志目录(目录必须存在)。推荐设置，可以有效定位问题。（可实现为程序配置项）
  const char* log_path = “./log”;
if( efs->setOption( Dahua::EFS::efsLogOutput, log_path, strlen( log_path ) ) ){
  	std::cout << “set log path” << std::endl;
  }else{
  	std::cout << “set log path failed” << std::endl;
  }
  // 设置日志级别，对接过程中推荐设置为DEBUGF。（可实现为程序配置项）
  // 可选值由Defs.h中的日志级别的枚举确定
  int32_t log_level = Dahua::EFS::DEBUGF;
  if( efs->setOption( Dahua::EFS::efsLogLevel, &log_level, sizeof( log_level ) ) ){
  	std::cout << “set log level” << std::endl;
  }else{
  	std::cout << “set log level failed” << std::endl;
  }
  ```

* 提示

  * 推荐设置日志目录和级别，方便定位问题。日志文件占用最大空间为30*50MB，会自行覆盖。对接测试时建议设置为DEBUG，发布时可配置为INFO。因有改动需求，建议实现为配置项。
  * efsConcurrent默认为8，为测试时最佳性能值，一般无需修改
* efsBufferSize默认8个条带的大小，大文件的默认缓存是8*32K*(N+M)，小文件的默认条带是8*4K*(N+M)
  
  * 其他配置一般无需设置，可维持默认
* Windows下设置日志目录时请使用Linux格式，即选择以”/”作为目录分割符，比如d:/a/b/c

###### ~~监听EFS连接状态~~

`bool CEFileSystem::setListener(IEFSListener* listener);`

* 接口参数
  listener：

  ```C++
  //[IN ] 文件系统监听者，为NULL时表示删除监听。
  //IEFSListener内部有一个虚函数
  virtual void onEFSStateChanged(CEFileSystem* efs, EFSState state)
  //其中，efs为需要设置的文件系统实例，state表示需要监控的文件系统状态。
  enum EFSState {
  efsStateInit,			//初始状态
  efsStateNormal,		//efs客户端正常
  efsStateException	//efs客户端异常，会进行自动恢复
  efsStateError   ///<efs客户端错误，可能是用户不存在或者密码错误，不可自动恢复，需要用户介入.
  };
  ```

* 接口返回值

* 示例

  ```C++
  // 继承自Dahua::EFS::IEFSListener，实现EFS状态处理
  class EFSListener : public Dahua::EFS::IEFSListener
  {
  	public:
  	virtual void onEFSStateChanged( Dahua::EFS::CEFileSystem* efs, Dahua::EFS::EFSState state )
  	{
  		// 可根据EFS状态进行处理，
  		// 常规用法是：当EFS连接状态异常时，切换为其他存储模式，保证数据安全
  		std::cout << “EFS state is “ << state << std::endl;
  		return;
  	}
  }; 
  // 监听对象
  EFSListener* listener = new EFSListener;
  // 监听文件状态
  if( efs->setListener( listener ) ){
  	std::cout << “set efs listener” << std::endl;
  }else{
  	// 一般不会失败
  }
  // 如果与EFS连接状态出现变化，会调用EFSListener::onEFSStateChanged()
  // 设置为NULL，不再监听EFS状态
  if( efs->setListener( NULL ) ){
  	std::cout << “set efs listener to NULL” << std::endl;
  }else{
  	// 一般不会失败
  }
  
  // 销毁监听对象
  delete listener;
  listener = NULL;
  ```

* 提示

  * 7.2之后，允许在一个CEFileSystem对象上设置多个监听，但是一旦设为NULL则清除所有监听。
  * 一般场景下，无需使用该功能
  * 异步回调，请注意锁的使用，避免加锁调用CEFileSystem类的接口，避免死锁。

###### 设置用户空间紧急覆盖策略
功能：设置用户空间回收策略
`bool CEFileSystem::setRecyclePolicy( bool enableRecycle , RecyclePolicy policy);`

* 接口参数
  enableRecycle：  [IN ] 是否使能用户空间回收  
  policy：

  ```C++
  [IN ] 回收策略
  enum RecyclePolicy{
  	reduceByTime = 0,	///<等时间缩减
  	reduceByRatio = 1,	///<等比例缩减
  };
  ```

* 示例：

  ```C++
bool enable_recycle = true; // 使能紧急覆盖
  Dahua::EFS::RecyclePolicy policy = Dahua::EFS::reduceByTime; // 等时间删除
  // 设置紧急覆盖后，只对该用户下的设置了生命周期的Bucket起作用
  bool ret = efs->setRecyclePolicy( enable_recycle, policy );
  if( ret ){
  	std::cout << “set recycle policy” << std::endl;
  }else{
  	std::cout << “set recycle policy failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：

  * 紧急覆盖仅对使能生命周期的Bucket有效。默认情况下，用户的使用容量达到95%时，开始删除文件，释放空间。
* **等时间缩减**指该用户下所有Bucket删除<Bucket生命周期+固定时间>时间点前创建的文件
  
  * **等比例缩减**指该用户下所有Bucket删除<Bucket生命周期*固定比例>时间点前创建的文件

###### 获取用户空间紧急覆盖策略
`bool CEFileSystem::getRecyclePolicy( bool& enableRecycle, RecyclePolicy& policy);`

* 接口参数
  enableRecycle：  [OUT ] 是否使能用户空间回收 
  policy：

  ```C++
  [OUT ] 回收策略
  enum RecyclePolicy{
  	reduceByTime = 0,					///<等时间缩减
  	reduceByRatio = 1,					///<等比例缩减
  };
  ```

* 示例

  ```C++
bool enable_recycle; // 使能紧急覆盖
  Dahua::EFS::RecyclePolicy policy; // 等时间删除
  // 获取紧急覆盖策略
  ret = efs->getRecyclePolicy( enable_recycle, policy );
  if( ret ){
  	std::cout << “get recycle policy” << std::endl;
  }else{
  	std::cout << “get recycle policy failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* ~~提示~~

###### 退出EFS服务

功能：关闭文件系统
`bool CEFileSystem::close();`

* 提示
  * CEFileSystem::close()前，请务必保证通过该对象创建或打开的文件都已经关闭
  * 一般无需判断CEFileSystem::close()的返回值
  *  仅在确定后续不使用CEFileSystem实例时调用，比如进程退出前
  * 调用CEFileSystem::close()后，该CEFileSystem实例不可再使用

##### 管理Bucket

**允许多线程调用**，命名空间省略了Dahua::EFS

###### 创建Bucket
`CBucket CEFileSystem::createBucket( const char* name );`

* 接口参数
  name：[IN] bucket名称，必须符合命名规范

* 返回值
  CBucket对象示例。
  **必须CBucket:: isValid()来检测其有效性**，其返回false则创建失败。

* 示例

  ```C++
  const char* bucket_name = “bucket_test”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->createBucket( bucket_name );
  // 必须检查返回的CBucket对象是否有效，无效说明创建失败了
  if( bucket.isValid() ){
  	std::cout << “create bucket “ << bucket_name << std::endl;
  	// 操作CBucket对象
  	// ...
  }else if( -80005 == Dahua::EFS::getLastError() ){
  	// 错误码是-80005（Bucket已经存在），则需要通过获取Bucket接口获得CBucket对象
  }else{
  	std::cout << “create bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * 创建的Bucket的访问权限继承自存储用户的访问权限，可通过接口重新设置；
  * Bucket名全局唯一，需保证不与其他用户冲突；
  * 创建失败错误码-80005时，可直接获取已存在的Bucket；
  * 单个Bucket下的文件过多时，会影响读取性能，推荐每天创建不同的Bucket进行存储；
  * 创建bucket的数量不要超过运维上面创建用户时指定的bucket数量，不然会报-60010
  * 命名规范参考前面

###### 删除Bucket
`bool CEFileSystem::removeBucket( const char* name )`

* 示例

  ```C++
  const char* bucketName = “bucket1”; // bucket名必须符合命名规范
  if( efs->removeBucket( bucket_name ) ){
  	std::cout << “remove bucket “ << bucket_name << std::endl;
  }else{
  	std::cout << “remove bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：删除前，**必须先删除bucket下的所有文件**，否则错误码-80006

###### 获取Bucket
`CBucket CEFileSystem::getBucket( const char* name)`

* 返回值：CBucket对象实例，需要用 CBucket::isValid() 检测。

* 示例

  ```C++
  const char* bucket_name = “bucket1”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->getBucket( bucket_name );
  if( bucket.isValid() ){
  	std::cout << “get bucket “ << bucket_name << std::endl;
  	// 操作CBucket对象
  	// ...
  }else if( -80004 == Dahua::EFS::getLastError() ){
  	// 错误码是-80004（Bucketn不存在），则需要通过创建Bucket接口获得CBucket对象
  }else{
  	std::cout << “get bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：错误码时-80004时，表示Bucket不存在，需要先创建Bucket

###### 列出Bucket
`bool CEFileSystem::listBucket(CToken& token, uint32_t maxNumber, CNameList& list)`

* 接口参数

  * Token：[IN]  记录查询Bucket的分页信息
  * maxNumber：[IN] 最多可能列出的Bucket个数，建议取值小于1024
  * list：[OUT] 存放列出的bucket名

* 示例

  ```C++
  Dahua::EFS::CToken token;
  do{
  	// 接口内部会修改token的值，保证依次列出bucket
  	// 如果在执行的过程中有增删bucket的操作，那么此处列出的Bucket可能不准确。
  	Dahua::EFS::CNameList namelist; // 每次list前必须重新构造，否则可能包含重复的信息
  	if( efs->listBucket( token, 1024, namelist ) ){
  		// 输出bucket名字
  		std::cout << “list bucket, num:” << namelist.size() << std::endl;
  		for( int32_t I = 0; I < namelist.size(); ++I ){
  			std::cout << namelist.getName( I ) << std::endl;
  		}
  		
  		if( namelist.size() < 1024 ){
  			// bucket已经全部获取到
  			break;
  		}
  	}else{
  		// 获取失败，不再获取
  		std::cout << “list bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  		break;
  	}
  }while( 1 );
  ```

* 提示：list过程，若有删除操作，list可能有遗漏

###### ~~设置Bucket访问权限~~
`bool CBucket::setPrivilege(AccessPrivilege privilege)`

* 接口参数：
  Privilege

  ```C++
  //[IN] 需要设置的bucket权限。
  enum AccessPrivilege {
  	privilegePrivate= 0,		//私有，其他用户不可见
  	privilegeRead	= 1,		//只读，其他用户无创建、删除、写入权限
  	privilegeWrite	= 2		//读写，其他用户有创建、删除、写入权限
  };
  ```

* 示例

  ```C++
  const char* bucketName = “bucket_pri”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->createBucket( bucketName );
  // 检查bucket是否有效
  if( bucket.isValid() ){
  // 设置bucket的访问权限
  if( bucket.setPrivilege( Dahua::EFS::privilegePrivate ) ){
  	    std::cout << “set bucket privilege: Private” << std::endl;
  }else{
  	    std::cout << “set bucket privilege failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  }else{
  std::cout << “create bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：默认访问权限为存储用户的访问用户，一般不需要重新设置访问权限

###### 查询Bucket访问权限
`bool CBucket::getPrivilege(AccessPrivilege& privilege)`

* 示例

  ```C++
  const char* bucketName = “bucket_pri”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->createBucket( bucketName );
  // 检查bucket是否有效
  if( bucket.isValid() ){
  // 获取bucket的访问权限
  Dahua::EFS::AccessPrivilege pri;
  if( bucket.getPrivilege( pri ) ){
  		std::cout << “The privilege of “ << bucketName << “ is “ << pri << std::endl;
  	}else{
  		std::cout << “get privilege failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  }else{
  	std::cout << “create bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

###### ~~设置Bucket配额容量~~
`bool CBucket::setCapacity( uint64_t megaBytes )`

* 接口参数
  megaBytes：[IN] 需要设置的bucket容量大小，单位MB。**-1代表不限制配额**

* 示例

  ```C++
  const char* bucketName = “bucket_cap”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->createBucket( bucketName );
  // 检查bucket是否有效
  if( bucket.isValid() ){
  // 设置bucket配额容量为1024MB
  	if( bucket.setCapacity ( 1024 ) ){
  		std::cout << “set bucket capacity” << std::endl;
  	}else{
  		std::cout << “set bucket capacity failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	} 
  }else{
  	std::cout << “create bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * 默认是存储用户的剩余容量，同一用户下的Bucket抢占式使用存储用户的剩余容量。
  * 一般无需设置配额容量
  * 设置容量不得大于用户剩余容量

###### 查看Bucket配额容量

（获取bucket最大容量和已经使用的容量）

`bool CBucket::getCapacity(uint64_t& megaBytes, uint64_t& usedMegaBytes)`

* 接口参数

  * megaBytes：[OUT] 获取到的bucket最大容量，单位MB。
  * usedMegaBytes：[OUT] 获取到的bucket已经使用的容量，单位MB

* 示例

  ```C++
  const char* bucketName = “bucket_cap”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->getBucket( bucketName );
  // 检查bucket是否有效
  if( bucket.isValid() ){
  uint64_t totalCap = 0;
  uint64_t usedCap = 0;
  if( bucket.getCapacity (totalCap, usedCap) ){
  		std::cout << “total is “ << totalCap << “MB, used “ << usedCap << “MB” << std::endl;			
  }else{
  		std::cout << “get bucket capacity failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  }else{
  	std::cout << “get bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：未设置Bucket配额容量时调用该接口，则只有Bucket的已使用容量有效

###### 设置Bucket生命周期

```C++
bool CBucket::setLifeCycle(  
    int64_t delay, 
	bool enableAction, 
	bool removeWhenBucketEmpty,
	RecycleAction act=doDelete, 
	const char* prefix=NULL 
);
```

* 接口参数

  * delay：[IN ] bucket下文件的过期时间，单位为秒，最小值为3600秒

  * enableAction：[IN ] 是否使能参数act所指定的操作

  * removeWhenBucketEmpty：[IN ] 当bucket下无文件时，是否删除

  * act：[IN ] bucket过期时执行的操作，暂仅支持删除操作

    ```C++
    enum RecycleAction{
    	doDelete = 0						///<删除
    };
    ```

  * prefix：[IN ] 保留字段

* 示例

  ```C++
  const char* bucketName = “bucket_lifecycle”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->createBucket( bucketName );
  // 检查bucket是否有效
  if( bucket.isValid() ){
  	// 设置bucket生命周期，bucket下的文件从创建30天后删除，且当Bucket下无文件时，删除Bucket
  	int64_t delay = 30 * 24 * 3600;
  	bool enableAction = true;
  	bool removeWhenBucketEmpty = true;
  	Dahua::EFS::RecycleAction act = Dahua::EFS::doDelete;
  	const char* prefix = NULL; // 目前未使用必须为NULL
  	if( bucket.setLifeCycle( delay, enable_action, remove_when_bucket_empty, act, prefix ) ){
  		std::cout << “set bucket lifecycle” << std::endl;
  	}else{
  		std::cout << “set bucket lifecycle failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}	
  	// 取消Bucket生命周期
  	enable_action = false; // prefix必须为NULL，其他字段维持原来的值即可
  	if( bucket.setLifeCycle( delay, enable_action, remove_when_bucket_empty, act, prefix ) ){
  		std::cout << “cancle bucket lifecycle” << std::endl;
  	}else{
  		std::cout << “cancle bucket lifecycle failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	} 
  }else{
  	std::cout << “create bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：

  * 建议生命周期时间不少于1天，过短可能导致文件正在写入时被删除，会导致数据丢失。
  * **enableAction设置为false，不会执行删除文件的操作，生命周期不会起作用，并且设置的delay也不会生效。**

###### 查询生命周期

```C++
bool CBucket::getLifeCycle(  int64_t& delay, 
	bool& enableAction,
	bool& removeWhenBucketEmpty, 
	RecycleAction& act=doDelete,
	const char* prefix=NULL 
);
```

* 接口参数：同上

* 示例

  ```C++
  if( bucket.getLifeCycle( delay, enable_action, remove_when_bucket_empty, act, prefix ) ){
  		std::cout << “get bucket lifecycle” << std::endl;
  	}else{
  		std::cout << “get bucket lifecycle failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	} 
  ```

###### 获取Bucket下文件个数
`bool CBucket:: getFileNum( uint64_t& bigFileNum, uint64_t& smallFileNum )`

* 接口参数

  * bigFileNum [OUT]
  * smallFileNum [OUT]

* 示例

  ```C++
  ...
  	uint64_t big_file_num = 0;
  	uint64_t small_file_num = 0;
  	if( bucket.getFileNum( big_file_num, small_file_num ) ){
  		std::cout << “big file num:”<< big_file_num << “ small file num:” << small_file_num << std::endl;
  	}else{
  		std::cout << “get bucket file num failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  ...
  ```

* 提示

  * 因EFS延时删除文件的机制，该接口获取的文件数是有延迟的，即文件删除后，通过该接口获取的文件数不能及时反映。**请不要在类似“当文件数达到一定值时，开始删除文件“这样的逻辑中使用该接口。**
  * 该接口**获取的小文件个数并不准确**，实际返回的是**Container的个数**，请**不要依赖该数值做逻辑处理**。每个Container可能包含多个小文件。

##### 管理File

**同一个文件对象不支持多线程并发调用其接口**，命名空间Dahua::EFS

###### 创建文件

```C++
CFile CEFileSystem::create(  const char*  filename, 
	uint16_t n = 4, 
	uint16_t m = 1, 
	bool bigFile = true 
);
```

* 接口参数

  * filename：[IN] 必须是bucketname/filename的形式，必须符合命名规范
  * n： [IN] 冗余规则N+M中的N，默认是4
  * m：[IN] 冗余规则N+M中的M，默认是1
  * bigFile：[IN] 是否大文件，true表示创建大文件，false表示创建小文件，默认值true。（建议：100M以下的建议小文件，100M以上的建议大文件）

* 返回值：CFile对象，需用 CFile::isValid() 检测

* 示例

  ```C++
  // 创建文件时，必须保证Bucket已存在
  const char* bucket_name = “bucket1”; 
  // 创建时指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 以4+1模式创建大文件
  Dahua::EFS::CFile file = efs->create( filename.c_str(), 4, 1, true );
  // 必须检查返回的CFile对象是否有效，无效说明创建失败了
  if( file.isValid() ){
  	// 文件创建成功，该文件只能用于写
  
  	// 结束写后，必须关闭文件
  	file.close();
  }else{
  // 如果最后错误码是-80003，则说明文件已经存在
  	std::cout << “create file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * Bucket必须已存在
  * **建议将N+M实现为可配置项**
  * 获取到的CFile对象只可用于写入
  * 文件名不区分大小写，建议小写
  * 小文件写入不得超过100MB，大文件不得超过2TB
  * 命名规范见 使用说明-File

* ==创建模拟目录==
  EFS中不存在目录概念，但可通过该接口创建一个模拟目录，本质上是创建了一个size为0的文件，该文件也可进行读写，但**不推荐对该文件进行读写**。创建模拟目录时，只需要**filename以’/’结尾即可**。

###### 打开文件
`CFile CEFileSystem::open( const char* filename, FileMode mode )`

* 接口参数

  * mode：[IN] 打开模式 。
    Dahua::EFS::fileModeRead 读打开
    Dahua::EFS::fileModeAppend  追加写打开（**暂时不支持**）

* 返回值，同创建文件

* 示例

  ```C++
  const char* bucket_name = “bucket1”; 
  // 打开时指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 读打开文件
  Dahua::EFS::CFile file = efs->open( filename.c_str(), Dahua::EFS::fileModeRead );
  // 必须检查返回的CFile对象是否有效，无效说明打开失败了
  if( file.isValid() ){
  	// 对文件进行读取操作
  	
  	// 关闭文件
  	(void)file.close();
  }else{
  	std::cout << “open file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：文件必须已存在，获取到的对象只可用于读取或者追加写入，取决于mode

###### 上传文件

写入指定长度的数据

`int CFile::write( const char* buf, uint32_t len )`

* 接口参数

  * buf：[IN] 需要写入文件的数据，空间由调用者申请和释放。buf不能为NULL
  * len：[IN] 需要写入的数据字节数，len必须大于0

* 返回值

  * \>0：成功写入的字节数
  * 0：暂时不可写，**请在休眠一段时间后，再次调用本接口**
  * -1：写入出错。可以调用getLastError()获取错误码

* 示例

  ```C++
  // 创建文件时，必须保证Bucket已存在
  const char* bucket_name = “bucket1”; // bucket名必须符合命名规范
  // 创建时指定的文件名，必须是包含有bucket名字
  std::string filename = bucket_name + “/” + “helloworld”;
  // 以4+1模式创建大文件
  Dahua::EFS::CFile file = efs->create( filename.c_str(), 4, 1, true );
  // 或者可通过CFile::open接口以追加写模式打开文件（文件必须已经存在），进行追加写入
  // Dahua::EFS::CFile file = efs->open( filename.c_str(),Dahua::EFS::fileModeAppend );
  // 必须检查返回的CFile对象是否有效，无效说明创建失败了
  if( file.isValid() ){
  	// 文件创建成功，该文件只能用于写
  	uint32_t want_write_len = 300 << 20; // 期望写入300M
  	uint32_t write_once_len = 32768*4*5; // 推荐每次写入的长度是32K*N的倍数，此处N是4，倍数设置为5
  	char* buf = new char[write_once_len]; // 待写入数据的缓存
  	uint32_t write_once_offset = write_once_len; // 初始设为write_once_len，保证进入读取原始数据的分支
  	uint32_t write_len = 0; // 已写入的总长度
  	while( write_len < want_write_len ){
  		if( write_once_offset == write_once_len ){
  			write_once_offset = 0;
  			uint32_t left_write_len = want_write_len – write_len;
  			// 每次写的长度不能超过剩余需要写的长度
  			write_once_len = write_once_len > left_write_len ? left_write_len : write_once_len;
  			// 读取原始数据，此处以memset模拟
  			memset( buf, 0, write_once_len );		
  		}
  		int ret = file.write( buf + write_once_offset, write_once_len – write_once_offset );
  		if( 0 < ret ){
  			write_once_offset += ret;
  			write_len += ret;	
  		}else if( 0 == ret ){
  			// 休眠1ms，必须有休眠，否则可能导致CPU占用上升
  			SLEEP_MS( 1 );
  		}else if( -1 == ret ){
  			std::cout << “write failed, error:” << Dahua::EFS::getLastError() << std::endl;
  			break;
  		}else{
  			// 目前不存在其他返回值
  			assert( 0 );
  		}	
  	}
  	// 结束写后，必须关闭文件
  	file.close();
  	delete[] buf;
  	buf = NULL;
  	std::cout << “write over, write length:” << write_len << std::endl;
  }else{
  	std::cout << “create file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * 该接口为**同步非阻塞接口**，并非一定写入指定的len长度的数据，实际写入长度以返回值为准
  * 大小文件只在创建时有区别，写入流程是一样的
  * 写入数据会先保存在本地内存中，**达到一个条带大小后，才会真正写入云存储**。异常情况下可能丢失本地内存中的数据。
  * 调用接口时，每次写入的长度推荐为**32K*N的倍数**
  * 可通过CFile::open()接口以追加写模式打开文件（文件必须已经存在），然后上传数据。使用追加写前，必须确认所使用的版本是否支持该功能（7.0版本之后支持）。
  * 不要对一个文件多线程调用该接口，除非您能保证数据写入的顺序

  

###### 下载文件
`int CFile::read( char* buf, uint32_t len );`

* 接口参数

  * buf：  ==[IN]== （out？）从文件中读出数据放入buffer，buf的空间由外部申请和释放。  
  * len：  [IN] 需要读到的字节数。  

* 返回值

  * \>0：实际读取的字节数
  * 0：展示不可读，**建议休眠一段时间后，调用该接口**
  * -1：读取错误
  * -2：读到文件末尾

* 示例

  ```C++
  const char* bucket_name = “bucket1”; 
  // 打开时指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 读打开文件
  Dahua::EFS::CFile file = efs->open( filename.c_str(), Dahua::EFS::fileModeRead );
  // 必须检查返回的CFile对象是否有效，无效说明打开失败了
  if( file.isValid() ){
  	// 文件有效，开始读取
  	uint32_t want_read_len = 10 << 20; // 期望读取10M
  	uint32_t read_once_len = 32768*4*5; // // 推荐每次读取的长度是32K*N的倍数，此处N是4，倍数设置为5
  	char* buf = new char[read_once_len]; // 读取数据的缓存
  	uint32_t read_len = 0;
  	// 读取指定长度
  	while( read_len < want_read_len ){
  		int ret = file.read( buf, read_once_len );
  		if( 0 < ret ){
  			read_len += ret;
  			uint32_t left_read_len = want_read_len – read_len;
  			// 每次读取的长度不能超过剩余需要读取的长度
  			read_once_len = read_once_len > left_read_len ? left_read_len : read_once_len;
  			// 对读取到的数据进行操作，比如写入本地文件
  			// 读取多少就写入多少
  			// write( fd, buf, ret );		
  		}else if( 0 == ret ){
  			// 休眠1ms，必须有休眠，否则可能导致CPU占用上升
  			SLEEP_MS( 1 );
  		}else if( -1 == ret ){
  			std::cout << “read failed, error:” << Dahua::EFS::getLastError() << std::endl;
  			break;
  		}else if( -2 == ret ){
  			// 读取到文件尾
  			std::cout << “read end” << std::endl;
  			break;
  		}
  	}
  	// 打开文件成功后，关闭文件
  	file.close();
  	delete[] buf;
  	buf = NULL;
  	std::cout << “read over, read length:” << read_len << std::endl;
  }else{
  	std::cout << “open file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * 接口为**同步非阻塞接口**，实际读取长度以返回值为准。
  * 若为边写边读，因写入机制，可能使得最后写入不满一个条带的数据可能无法立即读取，需要关闭文件后才能读取到
  * 边写边读时，立即写入立即读取有可能会读取失败，需要确保写入一个条带的数据后（推荐至少写入1M数据）再开始读取数据
  * 结合CFile::seek()，随机读取
  * **不要多线程调用该接口**

###### 关闭文件
`bool CFile::close()`

* 示例

  ```C++
  const char* bucket_name = “bucket1”; 
  // 打开时指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 读打开文件
  Dahua::EFS::CFile file = efs->open( filename.c_str(), Dahua::EFS::fileModeRead );
  // 必须检查返回的CFile对象是否有效，无效说明打开失败了
  if( file.isValid() ){
  	// 文件创建或打开成功后，必须关闭文件，一般无需关心关闭的结果
  	(void)file.close();
  }else{
  	std::cout << “open file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：调用关闭后，不得调用该文件对象的任意接口

###### 删除文件
`bool CEFileSystem::remove( const char* filename )`

* 接口参数

  * filename

* 示例

  ```C++
  const char* bucket_name = “bucket1”; 
  // 指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  	
  if( efs->remove( filename.c_str() ) ){
  	std::cout << “remove file” << std::endl;
  }else{
  	std::cout << “remove file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：删除前必须确保文件已经关闭

###### 锁定文件
`bool CEFileSystem::lockFile( const char* filename)`

* 接口参数：filename

* 示例

  ```C++
  // 锁定文件后，该文件无法被删除，但可以被重命名
  if( efs->lockFile( filename.c_str() ) ){
  	std::cout << “lock file” << std::endl;
  }else{
  	std::cout << “unlock file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

###### 解锁文件
`bool CEFileSystem::unlockFile( const char* filename)`

* 接口参数：filename

* 示例

  ```C++
  // 解锁文件后，文件可被删除
  if( efs->unlockFile( filename.c_str() ) ){
  	std::cout << “unlock file” << std::endl;
  }else{
  	std::cout << “unlock file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

###### 获取文件锁定状态
`int32_t CEFileSystem::getFileLockStat(const char* filename)`

* 返回值

  * 1：文件已锁定
  * 0：文件未锁定
  * -1：获取状态失败

* 示例

  ```C++
  const char* bucket_name = “bucket1”; 
  // 指定的文件名，必须包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 获取文件锁定状态
  int32_t state = efs->getFileLockStat( filename.c_str() );
  if( 1 == state ){
  	// 文件已锁定
  	std::cout << “file locked” << std::endl;
  }else if( 0 == state ){
  	// 文件未锁定
  	std::cout << “file not lock” << std::endl;
  }else if( -1 == state ){
  	// 获取失败
  	std::cout << “get lock state failed” << std::endl;
  }else{
  	// 无其他错误码
  	assert( 0 );
  }
  ```

###### 重命名文件
`bool CEFileSystem::rename(const char* oldFileName, const char* newFileName)`

* 接口参数

  * oldFileName：[IN] 旧文件名字，该文件名的组成是bucket名+’/’+真实文件名
  * newFileName：[IN] ......

* 示例

  ```C++
  const char* bucket_name = “bucket1”; 
  // 指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string old_filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 新文件名的Bucket必须与老文件名的Bucket一致
  std::string new_filename = std::string( bucket_name ) + “/” + “byeworld”;
  if( efs->rename( old_filename.c_str(), new_filename.c_str() ) ){
  std::cout << “rename file” << std::endl;
  }else{
  std::cout << “rename file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * 重命名时要保证文件已关闭
  * **新旧文件名bucket必须一致，不允许跨bucket重命名**

###### 监听文件状态
`bool CFile::setListener(IFileListener* listener)`

* 接口参数

  * listener

    ```C++
    //[IN ] 文件监听者，为NULL时表示删除监听。
    //IFileListener内部有一个虚函数，其中file为文件对象，state表示该文件对象的状态。
    virtual void onFileStateChanged(CFile* file, FileState state)
    enum FileState {
    	fileStateInit,				//初始状态
    	fileStateNormal,			//文件正常
    	fileStateWarning,			//文件有风险
    	fileStateException			//文件异常
    };
    ```

* 示例

  ```C++
  // 继承自Dahua::EFS::IFileListener，实现文件状态处理
  class FileListener : public Dahua::EFS::IFileListener
  {
  	public:
  		virtual void onFileStateChanged( Dahua::EFS::CFile* file, Dahua::EFS::FileState state )
  		{
  			// 可根据文件状态进行处理，常规只处理Dahua::EFS::fileStateWarning状态
  			std::cout << “file state is “ << state << std::endl;
  			if( Dahua::EFS::fileStateWarning == state ){
  				// 该状态说明N+M写入时，小于等于M个Block写入异常，
  				// 为保证数据冗余度，可选择设置关闭文件的标识，
  				// 写入线程检查该标识，关闭文件并重新创建文件写入
  				// 无特殊需求则不必使用该功能
  			}
  			return;
  		}
  };
  // 创建文件时，必须保证Bucket已存在
  const char* bucket_name = “bucket1”; 
  // 创建时指定的文件名，必须是包含有bucket名字，注意要有斜杠’/’分割
  std::string filename = std::string( bucket_name ) + “/” + “helloworld”;
  // 以4+1模式创建大文件
  Dahua::EFS::CFile file = efs->create( filename.c_str(), 4, 1, true );
  // 必须检查返回的CFile对象是否有效，无效说明创建失败了
  if( file.isValid() ){
  	// 监听对象
  	FileListener* listener = new FileListener;
  	// 监听文件状态
  	if( file.setListener( listener ) ){
  		std::cout << “set file listener to NULL” << std::endl;
  	}else{
  		// 一般不会失败
  	}		
  	// 对文件进行操作,如果在此过程中文件状态出现变化，会调用FileListener::onFileStateChanged()
  
  	// 不再监听文件状态
  	if( file.setListener( NULL ) ){
  		std::cout << “set file listener to NULL” << std::endl;
  	}else{
  		// 一般不会失败
  	}
  //销毁监听对象
  delete listener;
  	listener = NULL;
  
  	file.close();
  }else{
  	std::cout << “create file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * **不能直接在onFileStateChanged()中关闭文件，否则可能导致异常**
  * 不论读写都可监听文件状态变化
  * 7.2版本开始支持设置多个监听对象，但设置为NULL时，会取消全部监听
  * 除非确实需要对Dahua::EFS::fileStateWarning状态进行处理，否则一般情况下，只需根据CFile对象的接口返回值进行业务处理
  * 由于是异步回调，请注意锁的使用，避免死锁

###### 获取文件属性
`bool CFile::stat( FileStat& stat )`

* 接口参数：

  * stat

    ```C++
    [OUT] 文件属性
    typedef struct FileStat
    {
    	uint8_t mode; ///<读写模式，值为Dahua::EFS::FileMode
    	uint8_t isAppend; ///<1-正在写入，0-未在写入
    	uint8_t dataNum;	///<数据分片数，即N+M中N的值	
    	uint8_t parityNum;	///<冗余分片数，即N+M中M的值	
    	uint32_t	cTime; ///<创建时间
    	uint32_t	mTime; ///<最后修改时间	
    	uint64_t	totalSize;	 ///<文件大小
    char reserved2[108];
    }FileStat;
    ```

* 示例

  ```C++
  // 读打开文件
  Dahua::EFS::CFile file = efs->open( filename.c_str(), Dahua::EFS::fileModeRead );
  // 必须检查返回的CFile对象是否有效，无效说明打开失败了
  if( file.isValid() ){
  	// 获取文件状态
  	Dahua::EFS::FileStat s;
  	if( file.stat( s ) ){
  		std::cout << “get file stat:” << std::endl;
  		std::cout << “mode:” << s.mode << std::endl;
  		std::cout << “writing ? “ << ( 0 == s.isAppend ? “false” : “true” ) << std::endl;
  		std::cout << “N+M is “ << s.dataNum << “+” << s.parityNum << std::endl;
  		std::cout << “ctime: “ << s.cTime << std::endl;
  		std::cout << “mtime: “ << s.mTime << std::endl;
  		std::cout << “size: “ << s.totalSize << std::endl;
  	}else{
  		std::cout << “get file stat failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  }else{
  	std::cout << “open file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

###### ~~设置文件选项：设置CFile高级配置信息~~
`bool CFile::setOption(EFSOption key, const void* value, uint32_t len)`

* 接口参数

  * key

    ```C++
    //[IN ]系统配置选项。
    enum EFSOption
    {
    	efsRwMode			//读写模式(0异步1同步，默认为0)
    };
    ```

  * value：[IN ]对应配置项的值。

  * len：[IN]对应配置项value缓冲区的长度。单位byte。

* 示例

  ```C++
  	...
  	if( file.setOption( Dahua::EFS::efsRwMode, &option, sizeof( option ) ) ){
  		std::cout << “set file option” << std::endl;
  	}else{
  		std::cout << “set file option failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  	...
  ```

* 一般情况无需设置

###### ~~获取文件选项~~
`bool CFile::getOption(EFSOption key, void* value, uint32_t len)`

###### 修改文件偏移：根据指定基准位置定位到指定偏移
`bool CFile::seek( int64_t offset, int whence )`

* 接口参数

  * offset：[IN] 相对于whence位置后在文件中的偏移

  * whence：

    ```C++
    //[IN] 偏移的基准位置
    enum FileLocation
    {
    	efsBegin,      	//文件头位置
    	efsCurrent,       //当前文件位置
    	efsEnd			//文件尾位置
    };
    ```

* 示例

  ```C++
  ...
  // 必须检查返回的CFile对象是否有效，无效说明打开失败了
  if( file.isValid() ){
  	// 修改偏移
  	if( file.seek( 100, Dahua::EFS::efsBegin ) ){
  		std::cout << “seek to 100B” << std::endl;
  	}else{
  		std::cout << “file seek failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  ...
  ```

* 提示

  * 结合CFile::read()，可实现**随机读取**
  * 创建写和追加写模式的File不支持调用该接口，即**不支持随机写入**

###### 获取文件偏移：获取文件当前位置
`int64_t CFile::tell()`

* 返回值

  * \>=0：当前读到的位置相对于头文件的偏移
  *  <0 ：获取失败

* 示例

  ```C++
  // 必须检查返回的CFile对象是否有效，无效说明打开失败了
  if( file.isValid() ){
  	// 获取文件当前偏移
  	int64_t offset = file.tell();
  	if( 0 <= offset ){
  		std::cout << “current offset is “ << offset << std::endl;
  	}else{
  		std::cout << “get file current offset failed, error:” << Dahua::EFS::getLastError() << std::endl;
  	}
  	file.close();
  } ......
  ```

###### 分页查询Bucket下的文件名：

列出bucket内文件名落在（begin，end）范围内的文件名

`bool CBucket::list( const char* begin,const char* end, uint32_t maxNumber, CNameList& list );`

* 接口参数

  * begin：[IN] 起始文件名。无需包含Bucket名。如果begin为NULL，则从Bucket下具备最小值的文件开始查询，且包含该最小值的文件。
  * end：[IN] 结束文件名。无需包含Bucket名。如果end为NULL，则查询到该Bucket下具有最大值的文件。
  * maxNumber：[IN] 查询的最大文件个数。取值范围[1, 2048]
  * list：[OUT] 存放获取到的文件名

* 示例

  ```C++
  const char* bucket_name = “bucket_test”; // bucket名必须符合命名规范
  Dahua::EFS::CBucket bucket = efs->getBucket( bucket_name );
  if( bucket.isValid() ){
  	std::cout << “get bucket “ << bucket_name << std::endl;
  	// 假设该Bucket下存在文件 a,b,c,d
  	std::string begin_name = “b”; 
  	std::string end_name = “d”;
  	uint32_t max_num = 1024;
  	while( 1 ){
           Dahua::EFS::CNameList namelist;
  		if( bucket.list( begin_name.empty() ? NULL : begin_name.c_str(), end_name.empty() ? NULL : end_name.c_str(), max_num, namelist ) ){
  			// 这里将列出文件c，d，查询到的文件名不包含Bucket名
  			for( int I = 0; I < namelist.size(); ++I ){
  				std::cout << namelist.getName( I ) << std::endl;
  			}
  			if( max_num > namelist.size() ){
  				// 实际查询到的文件数小于指定的查询的个数，说明已经查询结束 
  				break;
  			}else{
  				// 可能还有文件，修改begin_name继续查询
  				begin_name = namelist.getName( namelist.size() – 1 );
  			}
  		}else{
  			std::cout << “list file failed, error:” << Dahua::EFS::getLastError() << std::endl;
  			break;
  		}
  	}
  }else{
  	std::cout << “get bucket failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示

  * 循环查找时，必须修改begin参数，否则总是返回第一次查询的结果
  * 可**参考示例**中的实现，如果begin和end为**空**时，**需要传递NULL**

###### 分页查询Bucket下的文件信息
`bool CBucket::list( const char* begin,const char* end, uint32_t maxNumber, CFileList& list )`

* 接口参数

  * 同上
  * list：[OUT] 存放获取到的文件信息。

* 示例

  ```C++
  if( bucket.list( begin_name.empty() ? NULL : begin_name.c_str(), end_name.empty() ? NULL : end_name.c_str(), max_num, filelist ) ){
  	// 这里将列出文件c，d，查询到的文件名不包含Bucket名
  	for( int I = 0; I < filelist.size(); ++I ){	
  		std::cout << “filename: “ << filelist.getName( I ) << std::endl;
  		std::cout << “size: “ << filelist.getFileSize( I ) << std::endl;
  		std::cout << “create time: “ << filelist.getCTime( I ) << std::endl;
  		std::cout << “modify time: “ << filelist.getMTime( I ) << std::endl;
  		std::cout << std::endl;
  	}
  }
  ```

* 提示：同上

###### 分页查询指定目录下的文件信息：

查询dir目录下的文件信息

`bool CBucket::list( const char* dir, const char* delimiter, const char* begin, uint32_t maxNumber,CFileList& list)`

* 接口参数

  * dir：[IN] 需要查找的文件夹路径，最后字符必须为’/’。无需包含Bucket名
  * delimiter：[IN] 文件夹路径分隔符，目前仅支持’/’

* 示例

  ```C++
  if( bucket.isValid() ){
  	std::cout << “get bucket “ << bucket_name << std::endl;
  	/**
  	 * 假设该Bucket下存在文件，a/, a/b, a/c/，a/c/d, a/e。可以认为其存在如下目录树
  	 * a/
  	 * |   |--b
  	 * |   |--c/
  	 * |       |--d
  	 * |   |--e
  	 */
  	std::string dir = “a/”;  // 必须以’/’结尾
  	std::string begin_name = “b”;
  	uint32_t max_num = 1024;
  	while( 1 ){
  		// 列出a/目录下的文件，最后一个字符必须是’/’
  		// 分隔符只支持’/’
  		// begin是”b”，因此查询的结果是c/,e
           Dahua::EFS::CFileList filelist;
  		if( bucket.list( dir.c_str(), “/”, begin_name.empty() ? NULL : begin_name.c_str(), max_num, filelist ) ){
  			// 这里将列出文件c/,e。查询到的文件名不包含目录名
  			// 不会递归查询子目录
  			for( int I = 0; I < filelist.size(); ++I ){
  				std::cout <<... filelist.getName( I ) << std::endl;
  				std::cout <<... filelist.getFileSize( I ) << std::endl;
  				std::cout <<... filelist.getCTime( I ) << std::endl;
  				std::cout <<... filelist.getMTime( I ) << std::endl;
  				std::cout << std::endl;
  			}
              ......
  ```

* 同上

##### 获取系统信息

###### 获取系统资源信息：

获取当前EFileSystem的信息，包括集群节点数，客户端运行个数等

`bool CEFileSystem::getSystemInfo( SystemInfo& info )`

* 接口参数

  * info

    ```C++
    //[OUT ]系统信息。
    typedef struct SystemInfo
    {
    	uint32_t	totalNodes;	///< 系统中的节点总数
    	uint32_t	totalClient;///< 系统中正在运行的总客户端数
    	uint32_t	curWriteFiles;///< 系统中正在写的文件数
    	uint64_t	totalFiles;	///< 用户的文件总数
    	uint64_t	totalSpace;///< 用户的总存储空间，单位MB
    	uint64_t	spaceLeft; ///< 用户真正可用的剩余存储空间，单位MB
    	char uuid[32]; ///< 系统uuid,32个字符
    	uint64_t spaceQuotaLeft; ///< 用户剩余可分配空间，单位MB
    	uint64_t spaceUsed; ///<用户已使用容量, 单位MB
    	char	reserved[44]; ///<保留
    }SystemInfo;
    ```

* 示例

  ```C++
  Dahua::EFS::SystemInfo info;
  bool ret = efs->getSystemInfo( info );
  if( ret ){
  	// 以下为所有用户的统计信息
  	std::cout <<... info.uuid << std::endl; // 系统唯一标识
  	std::cout <<... info.totalNodes << std::endl; // 系统中的总节点数
  	std::cout <<... info.totalClient << std::endl; // 系统当前正运行的客户端
  	std::cout <<... info.curWriteFiles << std::endl; // 系统当前正在写的文件总数
  	// 以下为当前用户的信息，请注意在系统实际容量不足时，spaceLeft+spaceUsed不一定等于totalSpace
  	std::cout <<... info.totalFiles << std::endl; // 当前用户的文件总数
  	std::cout <<... info.totalSpace << std::endl; // 当前用户的总空间
  	std::cout <<... info.spaceLeft << std::endl;  // 用户真正可用的剩余空间，可能是系统实际剩余空间
  	std::cout <<... info.spaceQuotaLeft << std::endl; // 剩余的可分配空间，即可以给Bucket的设定的最大配额
  	std::cout <<... info.spaceUsed << std::endl;  // 用户已经使用了的空间
  }else{
  	std::cout <<... Dahua::EFS::getLastError() << std::endl;
  }
  ```

* 提示：其中**totalFiles**并非大文件个数和小文件个数的总和，而是**大文件个数和container个数的总和**，而一个container可能包含多个小文件。**请不要在类似“当文件数达到一定值时，开始删除文件“这样的逻辑中依赖该数值。**

###### 获取系统支持的N+M
`bool CEFileSystem::getRedundanceCaps(CRedundanceCaps& caps);`

* 接口参数：caps

  ```C++
  //[OUT ]冗余信息。
  //其中CRedundanceCaps有三个方法:
  int CRedundanceCaps:: size()        //总共支持的组合类型
      uint16_t getDataNum(int index)     //第index个组合中的N
      uint16_t getParityNum(int index)    //第index个组合中的M
  ```

* 示例

  ```C++
  Dahua::EFS::CRedundanceCaps caps;
  bool ret = efs->getRedundanceCaps( caps );
  if( ret ){
  	std::cout << “support:” << std::endl;
  	for( int32_t I = 0; I < caps.size(); ++I ){
  		std::cout << caps.getDataNum( I ) << “+” << caps.getParityNum( I ) << std::endl;
  	}
  }else{
  	std::cout << “get n+m failed, error:” << Dahua::EFS::getLastError() << std::endl;
  }
  ```



### 常见问题

#### SDK问题定位流程

1.	查看SDK日志，./log 目录，日志名称是SDK_log_current.log
2.	查看日志里面的错误码

问题

* 初始化EFS失败
  * 服务器IP不对，查看运维或者ping检查
  * 用户名密码不对，错误码为-60000或-60001
* 创建文件失败 -40003
  * 代表系统不支持创建文件时指定的N+M。在未开启N+M:0时，必须保证系统中在线且能提供服务的存储节点个数大于等于N+M
* 读写失败 -60012
  * 表示用户设置的bucket容量大于用户剩余的容量，需要到云存储运维上面增加用户的容量
* 写文件失败 -70007
  * 这种情况一般出现在开启了N+M:0的环境中，因为系统中存储节点的磁盘个数小于N+M，所以写入失败
* 写入了数据，但无法读取到这部分数据
  * 由于SDK内部需要计算EC，只有当写入的数据**达到一个条带大小时，才会真正写入存储**（大文件的条带大小为32K\*N；小文件的条带大小为4K\*N，其中N为N+M的N）。
  * **对于大文件来说，一旦数据写入存储，就可以立即读取，但是对于小文件来说只有文件关闭后，才能读取。**
* 写文件时发现性能很差，带宽上不去
  * 对于业务层来说，其写入速度=当前带宽 * N/N+M，以3+1为例，当实际带宽为400MB/s时，业务层的写入速度应在300MB/s左右
  * 如果带宽无法跑满，请使用iperf等工具检测一下客户机到云存储所有节点（特别是存储节点）的实际带宽
* 删除了文件，但是获取系统资源信息时容量没有变化
  * 服务端会延时批量删除文件，以避免大批量删除文件时影响业务IO
  * **注意不要依赖该接口获取的容量信息来做循环覆盖功能，这可能导致多删除文件**。（使用紧急覆盖和Bucket生命周期等功能）
* 创建了一个文件，只写了几个字节，但是获取系统资源信息时发现容量少了好几百MB
  * 只创建文件不会占用系统容量，但是如果开始写数据，那么系统默认会预分配一个Object的空间，一个Object的大小为(N+M)*128MB，以4+1算，就是640MB。当文件关闭后，会重新计算该文件的实际占用空间。

### 错误码对照表

-10000: 失败
-10001: 到达文件尾
-10002: 文件无效
-10003: 参数错误
-10004: 文件模式错误
-10005: 操作不支持
-10006: offset非法
-10007: 最大客户端连接数
-10008: efs系统当前不为空
-10009: bucket无效
-10010: EOSClient与mds通讯异常
-10011: EOSClient内部错误
-10012: 最大DN错误数，N+M情况下，发生了大于M个错误
-10013: SDK没有准备就绪，发生在错误的调用上，如没有初始化就调用其他接口
-10014: EC算法错误
-10015: 元数据错误
-10016: EFSClient与catalogServer通讯异常
-10017: 超过小文件最大大小
-10018: 文件被截断，发生在数据已经写到dn，但sdk或mds截断文件的情况
-10019: 操作超出文件范围，如seek传入参数大于文件范围
-10020: container达到最大并发限制
-10021: 与dn通讯异常
-10022: 系统句柄受限导致错误
-10023: 操作失败，建议进行重试

-20000: 网络地址错误
-20001: 正常连接出现异常
-20002: server绑定监听失败
-20003: 接受连接异常
-20004: 发起连接时错误
-20005: 连接超时
-20006: 协议解析错误
-20007: 请求超时时间内得不到回复
-20008: 请求的消息不支持，即没有消息处理函数
-20009: 请求暂时不可发送，可重试

-30000: 打开文件错误
-30001: 读文件错误
-30002: 写文件错误
-30003: 文件忙
-30004: link错误
-30005: unlink错误
-30006: 关闭文件错误
-30007: 文件系统错误
-30008: 获取文件属性错误
-30009: 创建目录错误
-30010: 目录访问错误
-30011: 挂载错误
-30012: 无效的fd
-30013: seek文件失败
-30014: 同步文件到磁盘错误
-30015: EMFILE:Too
-30016: 文件截断错误

-40000: object collection个数超过系统限制
-40001: object collection大小超过系统限制
-40002: 文件类型不支持
-40003: 冗余规则不支持
-40004: object collection打开模式不支持
-40005: object collection处于写打开状态
-40006: object collection不处于写状态
-40007: object collection已经存在
-40008: object collection不存在
-40009: object collection租约不存在
-40010: Object分配失败，Object个数过多
-40011: Object分配失败，未修复的Object过多
-40012: 对象构造超时
-40013: 对象不存在
-40014: DN资源不足，导致object分配失败
-40015: object bucket不存在
-40016: object bucket非空
-40017: object bucket正在使用中
-40018: object bucket数目超出系统现在
-40019: 系统容量不足
-40020: object bucket已经存在
-40021: 刚提供服务时，dn还未准备就绪，需要sdk重试以避免错误
-40022: 网闸没有建立映射关系
-40023: 一个客户端打开的文件数过多
-40024: 一个客户端在配额不足的情况下连续创建的空文件数过多
-40025: 文件已损坏
-40026: 该汇报的object超过限制,溢出

-60000: 用户不存在
-60001: 用户密码错误
-60002: DN不合法
-60003: ClientName非法。
-60004: License不合法
-60005: 系统容量受到License限制
-60006: 当前MDS状态不允许此操作
-60007: 用户已经存在
-60008: 用户名非法
-60009: 用户操作无权限
-60010: 用户bucket个数上限错误，设置时达到用户上限，或者创建时达到用户配额
-60011: 用户bucket个数配额小于实际值
-60012: 用户bucket容量配额小于实际值
-60013: 用户数据非空，即存在bucket，此时不允许删除用户
-60014: 网络带宽限制

-70000: prepare阶段:发现当前session已经在写数据
-70001: 在write
-70002: 在write
-70003: 块索引不存在
-70004: 块写入长度溢出
-70005: 出现非追加写
-70006: 读偏移超过文件长度
-70007: block分配错误，空间不足
-70008: 无效的块索引
-70009: 磁盘列表为空
-70010: 所有磁盘的可用空间都不足
-70011: Datanode收到退出信号:准备退出
-70012: 设置块属性错误
-70013: prepare阶段，blocksize与DN配置文件中的块大小不一致
-70014: 写阶段的块没有正常关闭，该状态在读到块尾部的时候返回给sdk
-70015: 元数据中找不到附加属性的key/value键值对
-70016: datanode块个数限制
-70017: datanode存储层正在初始化
-70018: Block块恢复出错
-70019: Block块校验值错误
-70020: 块恢复:任务在recoverManager队列中就已超时
-70021: Datanode
-70022: 消息队列已满.SDK端应该重试
-70023: truncate或覆盖写block尾部数据时:无法重新计算尾部chkSumSize不对齐部分的CRC

-80001: 文件名称不合法
-80002: 文件不存在
-80003: 文件已经存在
-80004: Bucket不存在
-80005: Bucket已经存在
-80006: Bucket不为空
-80007: Bucket名称不合法
-80008: 文件容器不存在
-80009: 目录服务内部错误
-80010: catalogserver和mds不匹配
-80011: 文件数达到系统上限
-80012: 文件被锁定





### 练习

* 编译
  make clean && make demo

* 从编译服务器拷贝
  rm -rf EfsTools/* && scp -P 7298 -r 10.35.30.5:/root/EfsTools .

* 动态链接库环境变量

  export LD_LIBRARY_PATH=/home/yu_xuwei/libs/x86_64





### Todo

~~输入 退格键等字符处理~~

~~文件流~~

~~login error 判断~~

errorcode

~~want write length~~

~~bug 删除大小没减少~~  有延迟

~~bug 下载的文件大小比EFS的文件大~~

~~输入无回显~~

单元测试用例编写

文件流
https://blog.csdn.net/mercy_ps/article/details/81315544