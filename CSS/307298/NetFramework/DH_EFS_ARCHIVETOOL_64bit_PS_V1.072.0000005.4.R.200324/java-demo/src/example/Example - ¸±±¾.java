package example;

import java.util.ArrayList;
import java.util.List;

import com.dahuatech.archive.ArchiveReader;
import com.dahuatech.archive.ArchiveTool;
import com.dahuatech.archive.ArchiveWriter;
import com.dahuatech.archive.constants.EFSLogLevel;
import com.dahuatech.archive.struct.EFSConfig;
import com.dahuatech.efs.Bucket;
import com.dahuatech.efs.EFileSystem;
import com.dahuatech.efs.constants.EFSOption;

public class Example {
	// 根据实际环境，修改以下变量值
	private final static String address = "172.6.3.32"; // 云存储服务IP
	private final static String username = "root"; // 存储用户名
	private final static String password = "123456"; // 存储用户密码
	
	/**
	 * @brief 配置高级选项
	 * @param efs 需要配置高级选项的EFileSystem对象
	 */
	static void setEFSOption( EFileSystem efs )
	{
		// 设置日志目录(目录必须存在)。推荐设置，可以有效定位问题。（可实现为程序配置项）
		if( efs.setOptions( EFSOption.efsLogOutput, "./log/" ) ){
			System.out.println( "set log path" );
		}else{
			System.out.println( "set log path failed, error: " + EFileSystem.getLastError() );
		}
		
		// 设置日志级别，对接过程中推荐设置为DEBUGF,稳定后可设置为INFOF。（可实现为程序配置项）
		if( efs.setOptions( EFSOption.efsLogLevel, EFSLogLevel.DEBUGF ) ){
			System.out.println( "set log level" );
		}else{
			System.out.println( "set log level failed, error: " + EFileSystem.getLastError() );
		}
		
		// 其他高级选项类似，一般无需设置
		return;
	}
	
	/**
	 * @brief 初始化EFS SDK
	 * @retval null 初始化失败
	 * @retval 非null 已初始化的EFileSystem对象
	 */
	static EFileSystem initEFS()
	{
		// 小文件打包依赖于EFS SDK，先初始化EFileSystem
		EFileSystem efs = new EFileSystem();
		
		com.dahuatech.efs.struct.EFSConfig.ByReference cfg = new com.dahuatech.efs.struct.EFSConfig.ByReference();
		cfg.address = address; // EFS服务地址
		cfg.port = 38100; // EFS服务端口，固定不变
		cfg.userName = username; // 存储用户名
		cfg.password = password; // 存储用户密码
		
		if( !efs.init( cfg ) ){
			return null;
		}
		
		return efs;
	}
	
	/**
	 * @brief 创建指定名字的bucket
	 * @param efs 已初始化的CEFileSystem对象指针
	 * @param bucketName bucket名
	 * @retval true 成功
	 * @retval false 失败
	 */
	static boolean createBucket( EFileSystem efs, String bucketName )
	{
		Bucket bucket = efs.createBucket( bucketName );
		if( null != bucket ){
			// bucket不为null时，必须调用close()，否则会存在内存泄露
			bucket.close();
			return true;
		}
		
		if( -80005 == EFileSystem.getLastError() ){
			// -80004代表Bucket已存在，因此直接返回true
			return true;
		}

		return false;
	}
	
	/**
	 * @brief 初始化小文件打包工具
	 * @retval 非null 已初始化的ArchiveTool对象
	 * @retval null 失败
	 */
	static ArchiveTool initArchiveTool()
	{
		// 以下实际是初始化了小文件打包内部的EFileSystem,，
		// 由于最初接口设计时没有考虑复用外部的EFileSystem对象，导致还需要外部再次初始化
		ArchiveTool archive_tool = new ArchiveTool();
		
		EFSConfig.ByReference cfg = new EFSConfig.ByReference();
		cfg.address = address;
		cfg.port = 38100;
		cfg.userName = username;
		cfg.password = password;
		
		if( !archive_tool.init( cfg ) ) {
			return null;
		}
		
		return archive_tool;
	}
	
	/**
	 * @brief 以小文件打包方式上传文件
	 * @param writer 小文件打包写对象
	 * @retval 非null 上传成功的文件名
	 * @retval null 失败
	 */
	static String uploadFile( ArchiveWriter writer )
	{
		// 返回的文件名后缀为jpg
		if( !writer.open( "jpg" ) ){
			System.out.println( "open failed" );
			return null;
		}
		
		int want_write_len = 1 << 20; // 文件大小为1M
		int write_once_len = 12288; // 推荐每次写入N*4k倍数的数据，此处N为3
		byte[] buf = new byte[write_once_len]; // 待写入数据的缓存
		int write_once_offset = write_once_len; // 初始设为write_once_len，保证进入读取原始数据的分支
		int write_len = 0; // 已写入的总长度
		while( write_len < want_write_len ){
			if( write_once_offset == write_once_len ){
				write_once_offset = 0;
				int left_write_len = want_write_len - write_len;
				// 每次写的长度不能超过剩余需要写的长度
				write_once_len = write_once_len > left_write_len ? left_write_len : write_once_len;
				// 读取原始数据，此处以假数据模拟
				for(int i = 0; i < write_once_len; i++) {
					buf[i] = (byte)i;	
				}
			}
			int ret = writer.write( buf, write_once_offset, write_once_len - write_once_offset );
			if( 0 > ret ){
				System.out.println( "write failed" );
				break;
			}
			
			write_once_offset += ret;
			write_len += ret;	
		}
		
		// 打开文件成功后，必须关闭文件
		String filename = writer.close();
		// 写入全部数据才认为是成功，返回文件名，否则返回空
		return write_len == want_write_len? filename : null;
	}
	
	/**
	 * @brief 读取小文件打包方式写入的文件
	 * @param reader 小文件打包读取对象
	 * @param filename 待读取的文件名
	 * @retval true 成功
	 * @retval false 失败
	 */
	static boolean downloadFile( ArchiveReader reader, String filename )
	{
		long file_length = reader.open( filename );
		if( 0 == file_length ){
			System.out.println( "open failed" );
			return false;
		}
		
		// 可根据open()返回的length分配足够的空间，也可分配固定空间，进行分次读取

		int want_read_len = (int)file_length; // 小文件打包写入的文件不会超过int最大值
		int read_once_len = 12288; // 推荐每次读取的长度是4K*N的倍数，此处N是3
		byte[] buf = new byte[read_once_len]; // 读取数据的缓存
		int read_len = 0;
		while( read_len < want_read_len ){
			int ret = reader.read( buf, read_once_len );
			if( 0 < ret ){
				read_len += ret;
				int left_read_len = (int)(want_read_len - read_len);
				// 每次读取的长度不能超过剩余需要读取的长度
				read_once_len = read_once_len > left_read_len ? left_read_len : read_once_len;
				// 对读取到的数据进行操作，比如写入本地文件
				// 读取多少就写入多少
				// write( fd, buf, ret );		
			}else if( 0 == ret ){
				// 休眠1ms，必须有休眠，否则可能导致CPU占用上升
				try{
  					Thread.sleep(1);
  				} catch (InterruptedException e) {
  					e.printStackTrace();
  				};
			}else if( -1 == ret ){
				System.out.println( "read failed, error:" + EFileSystem.getLastError() );
				break;
			}else if( -2 == ret ){
				// 读取到文件尾
				System.out.println( "read end" );
				break;
			}
		}
		// 打开文件成功后，必须关闭文件.一般无需关心结果
		reader.close();
		
		// 读取到全部数据才认为是成功
		return read_len == want_read_len;
	}
	
	/**
	 * @brief 小文件打包写文件
	 * @param efs 已经初始化好的EFileSystem对象
	 * @param archiveTool 已经初始化好的ArchiveTool对象
	 */
	static List<String> archiveWrite( EFileSystem efs, ArchiveTool archiveTool )
	{
		ArchiveWriter writer = null;
		List<String> filenames = new ArrayList<String>(); // 保存文件名
		do{
			// 创建Bucket用于写入
			String bucket_name = "archive_test";
			if( !createBucket( efs, bucket_name ) ){
				System.out.println( "create bucket failed" );
				break;
			}
						
			writer = archiveTool.createArchiveWriter();
			if( null == writer ){
				System.out.println( "create archive writer failed" );
				break;
			}
			
			// 初始化，指定N+M和存储的bucket
			if( !writer.init( (byte)3, (byte)1, bucket_name ) ){
				System.out.println( "archive writer init failed" );
				break;
			}
			
			// 使用初始化好的ArchiveWriter对象上传文件.
			// 一个ArchiveWriter对象同一时刻只能单线程使用
			// 尽量复用一个ArchiveWriter对象进行写入
			int i = 10;
			while( 0 < i-- ){
				String filename = uploadFile( writer );
				if( null != filename ){
					System.out.println( "write file:" + filename );
					filenames.add( filename );
				}else{
					// 文件名为空说明写入失败
					System.out.println( "upload failed" );
				}
			}
		}while( false );
		
		if( null != writer ){
			// 当ArchiveWriter对象不再使用时，必须调用ArchiveWriter.release(),否则会存储在内存泄露
			writer.release();
		}
		
		return filenames;
	}
	
	/**
	 * @brief 读取小文件打包写入的文件
	 * @param efs 已经初始化好的CEFileSystem指针
	 * @param filenames 待读取的文件名
	 */
	static void archiveRead( ArchiveTool archiveTool, List<String> filenames )
	{
		ArchiveReader reader = archiveTool.createArchiveReader();
		if( null == reader ){
			System.out.println( "create ArchiveReader failed" );
			return;
		}
		
		// 根据文件名下载文件,使用同一个ArchiveReader读取同一bucket下相同前缀的文件，可获得性能提升。
		for( String filename : filenames ){
			if( downloadFile( reader, filename ) ){
				System.out.println( "read file:" + filename );
			}else{
				System.out.println( "read file failed:" + filename );
			}
		}
		// 当不再使用时，需要调用release()释放内部资源
		reader.release();
		
		return ;
	}
	
	public static void main(String[] args) 
	{
		
		EFileSystem efs = null;
		ArchiveTool archive_tool = null;
		boolean efs_inited = false;
		boolean archive_inited = false;
		do{
			// 小文件打包依赖于EFS SDK，先初始化EFileSystem
			efs = initEFS();
			if( null == efs ){
				System.out.println( "init efs failed, errno:" + EFileSystem.getLastError() );
				break;
			}
			efs_inited = true;

			// 初始化小文件打包
			archive_tool = initArchiveTool();
			if( null == archive_tool ){
				System.out.println( "init archive tool failed" );
				break;
			}
			archive_inited = true;
			
			List<String> filenames = archiveWrite( efs, archive_tool );
			archiveRead( archive_tool, filenames );
		}while( false );
		
		if( archive_inited ){
			archive_tool.close();
		}
		
		if( efs_inited ){
			efs.close();
		}
	}
}
