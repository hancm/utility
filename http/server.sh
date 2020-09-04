#! /bin/bash

proc=http
conf=./conf/http-config.xml

# echo颜色开始
ECHO_COLOR_BEGIN="\033["

# echo颜色结束
ECHO_COLOR_END="\033[0m"

# echo红底白字
ECHO_RED_WHITE_BEGIN="${ECHO_COLOR_BEGIN}41;37m"

# echo绿底白字
ECHO_GREEN_WHITE_BEGIN="${ECHO_COLOR_BEGIN}42;37m"

# 获取文本中数字
# 参数: 文本
# 返回: 文本中数字
function awk_number()
{
	number=`echo ${1} | awk -F "" '
	{
	  for(i=1;i<=NF;i++) 
	  {  
		if ($i ~ /[0-9]/)             
		{
		  str=$i
		  str1=(str1 str)
		}  
	  } 
	  print str1
	}'`
	
	echo ${number}
}

# 获取监听端口数
# 参数: 端口号(例如8081)
# 返回: 0/1
function listen_port_count()
{
	count=`netstat -pano | grep $1 | grep LISTEN | grep -v "grep" | wc -l`
	echo ${count}
}

#####################################################################################################################

# 获取conf中listening_ports的监听端口
# 默认8081
function grep_conf_listen_port()
{
	port_conf=`grep listening_ports ${conf}`
	echo $(awk_number ${port_conf})
}

#####################################################################################################################

# 根据端口号判断程序状态
function status()
{
	conf_port=$(grep_conf_listen_port)
	count=$(listen_port_count ${conf_port})
	if [ 0 == ${count} ];then
		echo -e "${ECHO_RED_WHITE_BEGIN}http server is not running${ECHO_COLOR_END}"
	else
		pid=`pidof ${proc}`
		if [ ${pid} ];then
			echo -e "${ECHO_GREEN_WHITE_BEGIN}port ${conf_port} is listening, http server is running pid: ${pid}${ECHO_COLOR_END}"
		else
			echo -e "${ECHO_RED_WHITE_BEGIN}port ${conf_port} is listening, but is not http server${ECHO_COLOR_END}"
		fi
	fi
}

# 根据监听端口(默认8081)判断启动程序
function start()
{
	count=$(listen_port_count $(grep_conf_listen_port))
	if [ 0 == ${count} ];then
		echo "#####start####"
		nohup ./${proc} -f ${conf} >/dev/null 2>&1 &
		sleep 1s
	fi
}

# kill程序proc
function stop()
{
	echo "#####stop####"
	pid=`pidof ${proc}`
	if [ "${pid}" ];then
		kill -9 ${pid}
	fi
	sleep 1s
}

function restart()
{
	stop
	start
}

########################################主程序#####################################
if [ "restart" = "${1}" ];then
	restart
	status
elif [ "stop" = "${1}" ];then
	stop
elif [ "status" = "${1}" ];then
	status
elif [ "start" = "${1}" ];then
	start
	status
else
	echo "Usage: ${0} [status] [start] [stop] [restart]"
fi

