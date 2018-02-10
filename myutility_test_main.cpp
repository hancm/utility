#include <iostream>

#include "File.h"
#include "log.h"
#include "MyLog.h"
#include "xmlConf.h"
#include "ZipSerialize.h"

static MyLog MLog("/tmp/MyLog.log");

int main(void)
{
	std::cout << "main test." << std::endl;
	
    // {}中要么同时加上参数位置:类型，要么都不加
    // {}个数可以和后面参数数目不同
    LOG_DEBUG("Main test {4:s} {2:f} {3:d}.", 123, "maintest", 1332.1340230, 'a', "oejeofjeofj", 22323);
    LOG_ERROR("Main test {} {} {}.", 123, "maintest", 1332.1340230, 'a', "oejeofjeofj", 22323);

	return 0;
}