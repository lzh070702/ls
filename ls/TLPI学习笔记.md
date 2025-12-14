# 函数
## 第4章 文件 I/O：通用的 I/O 模型
### open()函数
> open()调用既能打开一个业已存在的文件，也能创建并打开一个新文件。
1. 函数原型
```c
#include <sys/stat.h>
#include <fcntl.h>

int open(const char* pathname,int flags,mode_t mode);
```
2. 函数参数
* pathname：文件路径
* flags：打开方式
  * O_RDONLY：以只读方式打开文件（0）
  * O_WRONLY：以只写方式打开文件（1）
  * O_RDWR：以读写方式打开文件（2）
    > 三选一（必写）
  * ...（其他方式）
    > 多个方式之间用 | （按位与）拼接
* mode:文件权限
  > 仅在O_CREAT时填写（创建新文件时需要指明权限）
3. 函数返回值
* 成功：返回fd（>=3）
  > 文件描述符
  
  |文件描述符|用途|POSIX名称|stdio流|
  |:--:|:--:|:--:|:--:|
  |0|标准输入|STDIN_FILENO|stdin|
  |1|标准输出|STDOUT_FILENO|stdout|
  |2|标准错误|STDERR_FILENO|stderr|
* 失败：返回-1并设置errno
### read()函数
> read()系统调用从文件描述符 fd 所指代的打开文件中读取数据
1. 函数原型
```c
#include <unistd.h>

ssize_t read(int fd, void* buf, size_t count);
```
2. 函数参数
* fd：欲读取的文件描述符
* buf：存放输入数据的内存缓冲区地址
  > 缓冲区至少应有 count 个字节
* count：最多能读取的字节数
3. 函数返回值
* 成功：返回实际读取的字节数，如果遇到文件结束（EOF）则返回 0
* 失败：返回-1并设置errno
### write()函数
> write()系统调用将数据写入一个已打开的文件中
1. 函数原型
```c
#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count);
```
2. 函数参数
* fd：欲写入的文件描述符
* buf：要写入文件中数据的内存地址
  > 缓冲区至少应有 count 个字节
* count：欲从 buf 写入文件的数据字节数
3. 函数返回值
* 成功：返回实际写入的字节数，如果无数据写入则返回 0
* 失败：返回-1并设置errno
### close()函数
> close()系统调用关闭一个打开的文件描述符，并将其释放回调用进程，供该进程继续使用。
当一进程终止时，将自动关闭其已打开的所有文件描述符
1. 函数原型
```c
#include <unistd.h>

int close(int fd);
```
2. 函数参数
* fd:欲关闭的文件描述符
3. 函数返回值
* 成功：返回 0
* 失败：返回-1并设置errno
### lseek()函数
> lseek()系统调用依照 offset 和 whence 参数值调整该文件的偏移量
1. 函数原型
```c
#include <unistd.h>

off_t lseek(int fd,off_t offset,int whence);
```
2. 函数参数
* fd：欲调整偏移量的文件描述符
* offset：调整的偏移量
* whence：偏移量的参考基准
  * SEEK_SET：以文件起始位置为基准（0）
  * SEEK_CUR：以文件当前偏移量为基准（1）
  * SEEK_END：以文件末尾位置为基准（2）
3. 函数返回值
* 成功：返回相对文件起始位置的新偏移量
* 失败：返回-1并设置errno
## 第18章 目录与链接
### opendir()函数
> opendir()函数打开一个目录，并返回指向该目录的句柄，供后续调用使用
1. 函数原型
```c
#include <dirent.h>

DIR* opendir(const char * dirpath );
```
2. 函数参数
* path：目录路径
3. 函数返回值
* 成功：返回指向`DIR`结构体的指针

* 失败：返回`NULL`并设置errno
### readdir()函数
> readdir()函数从一个目录流中读取连续的条目
1. 函数原型
```c
#include <dirent.h>

struct direct* readdir(DIR* dirp);
```
2. 函数参数
* dirp:目录流句柄
  > 每调用 readdir()一次，就会从 dirp 所指代的目录流中读取下一目录条目，并返回一枚指针，
指向经静态分配而得的 dirent 类型结构，内含与该条目相关的如下信息：
  ```c
  struct direct
  {
      ino_t d_ino;    // i-node编号
      char d_name[];  // NULL字节结尾的文件名
  }
  ```
  > 每次调用 readdir()都会覆盖该结构。
3. 函数返回值
* 成功：返回指向当前目录项的struct dirent*指针
* 读取结束：返回NULL，且errno保持不变（需通过  errno  判断是“结束”还是“错误”）
* 失败：返回`NULL`并设置errno