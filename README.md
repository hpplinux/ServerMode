# 功能简介 #
基于libev的IO密集型服务程序的实现。
主线程 + 事件监听线程池 + 任务处理线程池 +其他的线程（日志等）
主线程用来一个事件循环监听新连接的到来。一旦连接成功就分发到事件监听线程池里。每个事件监听线程维护一个事件循环，用来监听客户端的请求，一旦客户端有请求到达就将请求信息封装成任务的形式交给任务处理线程池来处理。

项目中用到了CommonSDK这个库，这个基础库的也是自己封装的。里面大部分的功能都是基于boost封装的。都是一些常用的功能：
[https://github.com/fm123456/CommonSDK](https://github.com/fm123456/CommonSDK "CommonSDK")

# 使用 #
本模型具有很高的扩展性，在实际使用中我们只需要定义自己的TaskData（任务数据）和具体处理数据的task对象，重写execute方法就可以嵌入到该模型中。