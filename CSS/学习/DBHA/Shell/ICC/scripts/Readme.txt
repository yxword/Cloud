
说明：本文旨在说明 module-desc.xml描述文件 以及 DBHA-Shell-ICC-scripts 目录下每个脚本和配置文件的用途。
      该目录下的脚本和配置文件是基于ICC运维提供的模板而进行相应的修改。

脚本：
-service_manage_interface.sh
    服务管理接口脚本，ICC运维调用的脚本入口
    使用方法：
    1、更新配置：ICC运维调用该接口，将下发的json转成“key=value”格式的配置文件，然后使用该配置文件部署DBHA
       sh service_manage_interface.sh 1 "运维下发的json格式字符串"     
    2、读取配置：ICC运维读取本地的json模板或临时配置文件，目前暂不使用    
    3、停止服务
       sh service_manage_interface.sh 3       
    4、启动服务
       sh service_manage_interface.sh 4       
    5、查询服务状态
       sh service_manage_interface.sh 5       
    6、创建DB用户：密码策略为密码长度大于等于14位，需要有大小写，数字，特殊字符
       sh service_manage_interface.sh 6 username password
    98、初始化：将模板json文件中的值赋值到对应的配置项上，目前暂不使用
    99、重启服务
       sh service_manage_interface.sh 99


-config_processor.sh
    配置处理器脚本，定义了一些函数和变量，用于service_manage_interface.sh脚本调用
    函数说明：
    1、pre_processing：对运维下发下来的配置JSON进行预处理，生成"key=value"格式的键值对内容
    2、process_mapping_files：将预处理好的文件，根据key映射配置文件，得到本地key，从而生成部署DBHA时使用的“key=value”格式的配置文件
    3、update_config_from_runs：更新配置的入口，调用上述两个函数
    4、read_config_for_runs、read_current_config_for_runs：读取配置的入口，返回json模板内容或者根据已有的”key=value“格式的配置文件，生成的json内容，目前暂不使用
    5、log_out：打印日志到 ./service_manage.log
    
    
-user_defined.sh
    用户自定义接口脚本，定义了一些函数和变量，用于service_manage_interface.sh脚本调用
    函数说明：
    1、dbha_install：根据处理生成的配置文件,调用 VCS-DBHA.sh install，进行DBHA的部署并且拉起服务
    2、create_user：生成创建用户的配置文件，调用 VCS-DBHA.sh createUser 进行创建DB用户
    3、stop_service：停止服务，调用 VCS-DBHA.sh stop
    4、start_service：启动服务，调用 VCS-DBHA.sh start


-generate_config_json.py
    读取配置时调用的python脚本，返回json模板内容或者根据已有的”key=value“格式的配置文件生成的json内容，目前暂不使用
    


    
配置文件：
-properties_mapping.conf：下发配置项中本地key和ICC运维key映射的配置
    配置项说明：
    1、DBHA_virtual_IP=SERVER.DBHA.VIP  ：DBHA虚IP
    2、DBHA_hosts=SERVER.DBHA.BUSINESS.IP  ：DBHA业务IP
    3、DBHA_heartbeats=SERVER.DBHA.HEARTBEATS.IP  ：DBHA心跳IP


-properties_update.conf：用于部署DBHA的配置文件的更新策略的配置
    配置项说明：
    1、config-base-dir：配置文件路径
    2、scan-files-mapping：需要被扫描的“key-value”格式配置文件列表
    3、scan-files-placeholder：需要被扫描的占位符格式配置文件列表，目前暂不使用
    4、sensitive-field-mapping：“key-value”格式的敏感字段，目前暂不使用
    5、sensitive-field-placeholder：占位符格式的敏感字段，目前暂不使用
    
    
-module-dependence-config.json：json模板配置文件，用于上报给运维，目前暂不使用
    配置项说明wiki：http://10.1.253.177:8082/web/#/14?page_id=4832


-module-desc.xml：运维界面需要的xml描述文件
    配置项说明wiki：http://10.1.253.177:8082/web/#/96?page_id=14516
    
    
    
-完-
