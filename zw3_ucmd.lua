#!/usr/bin/lua

local json = require("luci.json")
local io = require("io")
local fs = require("nixio.fs")
local os = require("os")

local sys = require "luci.sys"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()

CODE_SUCCESS                  = 0      -- // 成功
CODE_NXP_SECURITY_HANDINGSHAKE= 96 	-- // NXP加锁和设备时，设备已经通讯上，正在进行加密握手操作
CODE_UPGRADE_STARTED          = 97     -- // 升级开始
CODE_UPGRADE_SUCCESS          = 98     -- // 升级成功
CODE_WAIT_TO_EXECUTE          = 99     -- // 命令已经收到，等待执行
CODE_UNKNOW_ERROR             = 199    -- // 未知错误
CODE_WRONG_FORMAT             = 101    -- // 报文格式错误
CODE_UNKNOW_DEVICE            = 102    -- // 未知的设备，指定的设备不存在
CODE_UNKNOW_ATTRIBUTE         = 103    -- // 未知的属性值
CODE_TIMEOUT                  = 104    -- // 操作超时
CODE_BUSY                     = 105    -- // 设备忙，已经有待执行的命令，且这个命令不能同时进行
CODE_UNKNDOW_CMD              = 106    -- // 未知的命令
CODE_NOT_SUPPORTED            = 107    -- // 不支持的操作
CODE_UPGRADE_MD5SUM_FAILED    = 108    -- // 升级时MD5SUM校验失败
CODE_UPGRADE_DOWNLOAD_FAILED  = 109    -- // 升级时固件下载失败
CODE_UPGRADE_FAILED           = 110    -- // 升级失败
CODE_PASSWORD_ALREADY_EXISTS  = 111    -- // 密码已经存在
CODE_PASSWORD_FULL            = 112    -- // 密码表已经满了
CODE_PASSWORD_NOT_EXISTS      = 113    -- // 要删除的密码不成功
CODE_MINUS_1                  = -1     -- // 未执行且不再执行的命令，由服务端更新

g_device_code	= ""
_gateway_device_id = nil
--: util function-------------------------------------------------------------------

function my_print(s)
	os.execute('logger -s \'' .. os.time() .. ' [INFO] ChenBei::' .. s .. '\'')
end

function ubus_send(to, msg)
  local pkt={PKT = json.encode(msg)}
  --my_print('ubus send DS.' .. string.gsub(json.encode(to),'%"', "") .. ' ' .. '\'' .. json.encode(pkt) .. '\'')
  --my_print('ubus send DS.' .. string.gsub(json.encode(to),'%"', "") .. ' ' .. '\'' .. json.encode(pkt) .. '\'')
  --os.execute('ubus send DS.' .. string.gsub(json.encode(to),'%"', "") .. ' \'' .. json.encode(pkt) .. '\'')
  my_print('ubus send DS.' .. to .. ' \'' .. json.encode(pkt) .. '\'')
  --os.execute('ubus send DS.' .. to .. ' \'' .. json.encode(pkt) .. '\'')
  luci.util.exec('ubus send DS.' .. to .. ' \'' .. json.encode(pkt) .. '\'')
end

function uptime() 
	file=io.open("/proc/uptime", "r")
	if (file == nil) then
		return "0"
	end
	ut = file:read("*n")
	-- my_print(ut)
	file:close()
	return math.floor(ut)
end
function get_version() 
	file=io.open("/etc/dusun_build", "r")
	if (file == nil) then
		return "unknown"
	end
	ver = file:read("*l")
	file:close()
	-- my_print(ver)
	ver = string.gsub(ver, "%s+", "")
	return string.gsub(ver, "BUILD_VERSION=", "")
end
function get_model()
	model=fs.readfile("/tmp/sysinfo/model")
	return  string.gsub(model, "%s+", "")
end
function get_factory()
	return "dusun"
end

function gwmac() 
	mac=fs.readfile("/sys/class/net/eth0/address")
	return  string.gsub(mac, "%s+", "")
end

function get_devcode() 
	if (g_device_code ~= "") then
		return g_device_code
	end
	file=io.open("/etc/dusun/device_code")
	if (file ~= nil) then
		g_device_code=fs.readfile("/etc/dusun/device_code")
		file:close()
	end
	return g_device_code
end

function set_devcode(deviceCode) 
	g_device_code = deviceCode
	fs.writefile("/etc/dusun/device_code", deviceCode)
end

function time() 
	return os.time()	
end

function get_gateway_device_id()
	-- cat /proc/sys/kernel/random/uuid
	-- return luci.sys.exec('cat /proc/sys/kernel/random/uuid')
	my_print('gwmac is ' .. gwmac())
	if (_gateway_device_id == nil) then
		t = io.popen('echo DusunGateway_' .. gwmac() .. ' | md5sum | xargs | cut -d " " -f 1')
		_gateway_device_id = 'ds' .. string.gsub(t:read('*all'), "%s+", "")
		t:close()
	end
	return _gateway_device_id
end
function get_gateway_model() 
	-- return luci.sys.exec('cat /tmp/sysinfo/model')
	-- return get_model()
	return 'hub_ds'
end
function get_gateway_version()
	-- return luci.sys.exec('cat /etc/dusun_build  | head -n1 | xargs | cut -d = -f 2')
	return get_version()
end
function get_gateway_battery() 
	return 100
end
function get_gateway_power_type()
	-- 0 battery, 1 outlet power
	return 1
end
function get_gateway_arm_mode() 
	-- 0 disarm, 1 arm home , 2 arm away
	-- return luci.sys.exec('[ -e /etc/config/dusun/chenbei/bufang ] && cat /etc/config/dusun/chebei/bufang || echo 0')
	t = io.popen('[ -e /etc/config/dusun/chenbei/bufang ] && cat /etc/config/dusun/chenbei/bufang || echo 0')
	x = t:read('*all')
	t:close()
	my_print('x is ' .. x)
	return tonumber(string.sub(x, 1, 1))
end
function set_gateway_arm_mode(armMode) 
	-- return luci.sys.call('echo ' .. armMode .. ' > /etc/config/dusun/chenbei/bufang')
	t = io.popen('echo ' .. armMode .. ' > /etc/config/dusun/chenbei/bufang')
	t:close()
	return 0
end
function get_gateway_sn() 
	-- mac without ':'
	-- return luci.sys.exec('cat /sys/class/net/eth0/address | sed "s/://g"')
	return gwmac()
end
function get_gateway_network_type() 
	-- 4G, wifi, eth
	-- return luci.sys.exec('[ -e /usr/bin/network_type.sh ] && /usr/bin/network_type.sh || echo eth')
	-- return 'eth'
	return string.gsub(luci.util.exec('[ -e /tmp/._network_type ] && cat /tmp/._network_type || echo eth'), "%s+", "")
end
function get_gateway_network_strength() 
	-- a value, 
	-- return luci.sys.exec('[ -e /usr/bin/network_strength.sh ] && /usr/bin/network_strngth.sh || echo 100')
	-- return 100
	return string.gsub(luci.util.exec('[ -e /tmp/._network_strength ] && cat /tmp/._network_strength || echo 100'), "%s+", "")
end
function get_gateway_network_id() 
	-- wifi -> ssid, 4g isp
	-- return luci.sys.exec('[ -e /usr/bin/network_id.sh ] && /usr/bin/network_id.sh || echo 0')
	-- return 'eth'
	return string.gsub(luci.util.exec('[ -e /tmp/._network_id ] && cat /tmp/._network_id || echo eth'), "%s+", "");
end

function gen_uuid() 
	return luci.util.exec('cat /proc/sys/kernel/random/uuid')
end

--------------------------------------------------------------------------------------------
function zwcmd(mac, ep, cmdid, data) 
	cmd = {
		data =  {
			arguments =  {
				attribute =  'ember.zb3.zclcmd',
				ep				= tonumber(ep, 16),
				mac				=	mac,
				value			= {
					ep				= ep,
					cmdid			= cmdid,
					data			= data
				}
			},
			command			=  'setAttribute',
			id					=  gen_uuid()
		},
		deviceCode		= 'db4a7b00-08f4-49e3-9ace-ac59ce035737',
		from					= 'CLOUD',
		mac						= gwmac(),
		time					= time(),
		to						= 'ZWAVE',
		type					= 'cmd'
	}
	ubus_send('ZWAVE', cmd)
end
function zwcmd_onoff(mac, ep, value) 
	cmd = {
		data =  {
			arguments =  {
				attribute =  'device.onoff',
				ep				= tonumber(ep, 16),
				mac				=	mac,
				value			= {
					ep			= ep,
					value		=  value
				}
			},
			command			=  'setAttribute',
			id					=  gen_uuid()
		},
		deviceCode		= 'db4a7b00-08f4-49e3-9ace-ac59ce035737',
		from					= 'CLOUD',
		mac						= gwmac(),
		time					= time(),
		to						= 'ZWAVE',
		type					= 'cmd'
	}
	ubus_send('ZWAVE', cmd)
end
function zwcmd_indicator(mac, ep, value) 
	cmd = {
		data =  {
			arguments =  {
				attribute =  'device.indicator',
				ep				= tonumber(ep, 16),
				mac				=	mac,
				value			= {
					ep			= ep,
					value		= value
				}
			},
			command			=  'setAttribute',
			id					=  gen_uuid()
		},
		deviceCode		= 'db4a7b00-08f4-49e3-9ace-ac59ce035737',
		from					= 'CLOUD',
		mac						= gwmac(),
		time					= time(),
		to						= 'ZWAVE',
		type					= 'cmd'
	}
	ubus_send('ZWAVE', cmd)
end

--zwcmd_indicator('30AE7B640DBA0909', 1, arg[1])
--zwcmd_onoff('30AE7B640DBA0909', 1, arg[1])

function usage() 
	print('zw3_cmd.lua help command shell')
	print(' usage: zw3_cmd.lua <function> <mac> <arg> <arg> ...!')
	print(' functions:"')
	print('  indicator <mac> <ep 1,2,3...> <0|1>             - alarm type set')
	print('  onoff <mac> <ep 1,2,3...> <0|1>                 - onoff')
end


luci.util.dumptable(arg)
print('argnum:' .. #arg)

if (arg[1] == 'help') then
	usage()
elseif (arg[1] == 'indicator') then
	if (#arg ~= 4) then
		usage()
		return
	end

	zwcmd_indicator(arg[2], arg[3], arg[4])
elseif (arg[1] == 'onoff') then
	if (#arg ~= 4) then
		usage()
		return
	end

	zwcmd_onoff(arg[2], arg[3], arg[4])
else
	usage()
end

