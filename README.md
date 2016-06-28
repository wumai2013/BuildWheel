# BuildWheel
A place belongs to the creation of the wheel。
#声明:由于本人英语极渣，所以大部分表述都是用中文来写。
Web4C简介:
	Web4C(Web for C),用C实现Web后台的一个框架。前端依旧是html+javascript+css,后端则是C语言。
简单的可以理解为CGI的一次实现。
	既然是造轮子，那当然是从头开始做起。为了测试的时候兼容浏览器，所以从WebServer做起，遵守HTTP协议。
	框架的基本架构:
		WebServer+Web4C-handler
		WebServer负责HTTP请求的处理，转发
		Web4C-handler负责HTTP请求后台的处理。