package com.example.helloworld;

import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;



@RestController
public class HelloController {
	
	@CrossOrigin(origins = "*")  // 允许来自所有域的跨域请求    //CORS跨域问题--虚拟机和其他
	 // 映射 /hello 路径并返回 "Hello World"
    @GetMapping("/hello")
    public String sayHello() {
        return "Hello World";
    }
}
