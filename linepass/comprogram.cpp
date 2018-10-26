//
//  comprogram.cpp
//  linepasswordindex
//
//  Created by Saber on 2018/5/14.
//  Copyright © 2018年 Saber. All rights reserved.
//

#include "comprogram.hpp"

client_config::client_config(std::map<std::string, std::string>* m) {
	init(m) ;
}

void client_config::init(std::map<std::string, std::string> * m) {
	connect_host = (*m)[CONNECTHOST] ;
	connect_port = atoi((*m)[CONNECTPORT].c_str()) ;
	connect_user = (*m)[CONNECTUSER] ;
	connect_password = (*m)[CONNECTPASSWORD] ;
}

ComProgram::ComProgram() {
	link = new LineLink(LineLink::CLIENT) ;
	cmd = new Command() ;
	ls = new LineSecret() ;
	//	lc = new ComLine() ;
}

ComProgram::~ComProgram() {
	delete link ;
	delete cmd ;
	delete ls ;
	ls = nullptr ;
	cmd = nullptr ;
	link = nullptr ;
}


bool ComProgram::connectServer() {
	/*
	 *	设置连接端口和ip
	 */
	link->clientPort(cc.connect_port) ;
	link->clientHost(cc.connect_host) ;
	
	
	if(!link->init()) return false;
	return link->clientConnect() ;
}


bool ComProgram::interactive() {
	struct proto_msg pm ;
	while (true) {
		if (cmd->input()) {
			if (cmd->cmd().local_type == type::quit) {
				return false ;
			} else {
				struct command cd = cmd->cmd() ;
//				char* tmp_c = (char*)malloc(sizeof(cd)) ;
//				memcpy(tmp_c, (char*)&cd, sizeof(cd)) ; // 以数据装配代替
				// 加密
//				const char* t1 = (const char*)cd.disassemble() ;
//				uint8_t* t2 = cd.disassemble() ;
				std::string cmd_str = ECB_AESEncryptStr(aesKey, (const char*)cd.disassemble(), 1030) ;
//				int8_t* t3 = (int8_t*)malloc(sizeof(int8_t) * (cmd_str.size() + 1)) ;;
//				memcpy(t3, cmd_str.c_str(), cmd_str.size()) ;
//				uint8_t* t4 = (uint8_t*)ECB_AESDecryptStr(aesKey,(const char*)t3).c_str() ;
//				printf("= %s\n\n%u,%u,%u\n",(int8_t*)cmd_str.c_str(),t1[1],t2[1],t4[1]) ;
//				printf("%s\n",cmd_str.c_str()) ;
				pm.server = COMMAND ;
				pm.len = cmd_str.size() ;
				pm.data = (int8_t*)cmd_str.c_str() ;
				
				uint32_t len = 0 ;
				uint8_t* pData = NULL ;
				pData = link->encode(pm, len) ;
				if(link->clientSend(pData, len)) {
					Server type ;
					unsigned int num = 0 ; // 返回结果数
					bool revcOk ; // 正常查询结果
					do {
						revcOk = link->clientRevc([&type,&num](struct proto_msg pm){
							type = pm.server ;
							switch(pm.server) {
								case Server::MESSAGE: {
									std::string back_str = ECB_AESDecryptStr(aesKey, (const char*)pm.data) ;
									printf("message: %s\n",back_str.c_str()) ;
									break ;
								}
								case Server::RESULT: {
									uint8_t* bdata = (uint8_t*)ECB_AESDecryptStr(aesKey, (const char*)pm.data).c_str() ;
									struct command cmd ;
									memcpy(&cmd, bdata, sizeof(command)) ;
									printf("   title : %s\n", cmd.ai.title) ;
									printf(" account : %s\n", cmd.ai.account) ;
									printf("  passwd : %s\n", cmd.ai.passwd) ;
									printf("nickname : %s\n", cmd.ai.nickname) ;
									printf(" company : %s\n", cmd.ai.company) ;
									num++ ;
									break ;
								}
								default:{
									break;
								}
							}
							return true ;
						}) ;
					} while(Server::RESULT == type) ;
					if (revcOk) {
						printf("-----------------------------\n") ;
						printf("Get %u result.\n\n",num) ;
					}
				}
			}
		} else {
			printf("command error!\n") ;
		}
	}
	return true ;
}




int ComProgram::main(int argc, char **argv) {
	
	
	cl->getKeyValue(argc, argv) ;
	cc.init(cl->map) ;
	hideArg(argc,argv,"line-passwd") ; // 隐藏密码
	
	/*
	 * 连接服务器，失败退出
	 */
	if (!connectServer()) {
		log("err: can not connect linepasswd server.") ;
		return 1 ;
	}
	
	/*
	 * 信息认证
	 */
	if(!certify(link)) {
		link->linkClose() ;
		log("err: certify.") ;
		return 1;
	}
	
	
	if (!interactive()) {
		link->linkClose() ;
		return 0 ;
	}
	return 0 ;
}

bool ComProgram::certify(LineLink* lk) {
	/*
	 *	转移用户信息
	 */
	struct user_config uc ; // struct.h
	struct proto_msg pm ; // link.hpp
	
	//	uc.user_user = cc.connect_user ;
	//	uc.user_password = cc.connect_password ;
	
	// 把用户信息复制到uc
	strcpy(uc.user_user, cc.connect_user.c_str()) ;
	strcpy(uc.user_password, cc.connect_password.c_str()) ;
	
	size_t size = sizeof(uc) ;
	char* plain = (char*)malloc(sizeof(char) * size) ;
	memcpy(plain,(char*)&uc,size) ;
	
	std::string data = ECB_AESEncryptStr(aesKey,plain,size) ;
	pm.data = (int8_t*)data.c_str() ;
	pm.len = data.size();
	pm.server = LOGIN ;
	uint32_t package_size;
	uint8_t* pdata = link->encode(pm, package_size) ;
	
	//	printf("发送密文:%s,长度:%d",pm.data,pm.len) ;
	
	/*
	 *	发送登录验证信息
	 */
	if(!lk->clientSend(pdata, package_size)) {
		return false;
	}
	
	/*
	 *	返回报文
	 */
	bool retVal = false ;
	lk->clientRevc([&](struct proto_msg pm) {
		try {
			if (LOGIN == pm.server) {
				const char* tmp_str = ECB_AESDecryptStr(aesKey, (const char*)pm.data).c_str() ;
				//			printf("接收密文:%s,长度:%d,解密:%s",pm.data,pm.len,tmp_str) ;
				if (!strcmp(tmp_str, CALLBACKOK)) {
					retVal = true ;
				}
			}
		} catch (Exception &e) {
			printf("%s\n",e.what()) ;
		}
	}) ;
	return retVal ;
}

int ComProgram::main() {
	return 0 ;
}
