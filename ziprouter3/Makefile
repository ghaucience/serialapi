##############################################
# OpenWrt Makefile for Z/IP Router program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to 
# specify a basic description of the package, 
# where to build our program, where to find 
# the source files, and where to install the 
# compiled program on the router. 
# 
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and 
# there should be no trailing whitespace in
# lines that are not commented.
# 
##############################################

include $(TOPDIR)/rules.mk
# Name and release number of this package
#VERSION_NAME:=$(VERISON_NAME)
PKG_NAME:=zipgateway
PKG_VERSION:=7.01.00
PKG_RELEASE:=1

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

# This specifies the directory where we're going to build the program.  
# The root build directory, $(BUILD_DIR), is by default the build_mipsel 
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR:=$(BUILD_DIR)/zipgateway-7.01.00

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk


# Specify package information for this program. 
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION 
# variable below and uncomment the Kamikaze define
# directive for the description below

#CONFIG_GCC_VERSION


define Package/zipgateway
	SECTION:=net
	CATEGORY:=Base system
	TITLE:=ZIP Router for Z-Wave Devices
	DEPENDS:=+kmod-tun +kmod-ipv6 +ip +libopenssl +libusb-1.0
endef

define Package/zipgateway_demo
	SECTION:=net
	CATEGORY:=Base system
	TITLE:=ZIP Demo gateway.
	DEPENDS:=+6scripts +6tunnel +dhcp6-client +dhcp6-server +gw6c +ip6tables \
		+zipgateway +luci-admin-full +luci-admin-mini +screen +nano +setserial \
		+radvd +netcat +tcpdump-mini +kmod-usb-serial +vpnc\
		+luci-medium +luci-admin-rpc +luci-app-firewall +luci-app-initmgr \
		+luci-app-livestats +luci-app-ntpc +luci-app-openvpn +luci-app-samba +luci \
		+kmod-usb-serial-pl2303 +kmod-usb-serial-cp210x +kmod-usb-serial-ftdi \
		+kmod-usb-serial-option +kmod-usb-acm +kmod-fs-ext2 +kmod-fs-ext3 +kmod-fs-isofs\
		+kmod-fs-mbcache +kmod-fs-ntfs +kmod-fs-udf +kmod-fs-vfat +kmod-nls-cp437 \
		+kmod-nls-cp850 +kmod-nls-cp852 +kmod-nls-iso8859-1 +kmod-nls-utf8 \
		+kmod-scsi-core +kmod-usb-storage 
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/zipgateway/description
	ZIP Router for Z-Wave Devices. Make a possible to Z-Wave Devices to reach internet.
endef

define Package/zipgateway_demo/description
	Virtial package which selects all pacakages needed for the demo firmware.
endef


# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r src/* $(PKG_BUILD_DIR)
	rm -rf $(PKG_BUILD_DIR)/obj_*
endef


# This helps to remove dead code from the binrary.
#
#TARGET_CFLAGS+=-fdata-sections -ffunction-sections
#TARGET_LDFLAGS+=--gc-
define Build/Compile
	$(call Build/Compile/Default, \
		CC=$(TARGET_CC) LD=$(TARGET_CC)  )
endef

define Package/zipgateway/install
	$(INSTALL_DIR) $(1)/usr/lib/lua $(1)/usr/bin $(1)/etc $(1)/etc/init.d $(1)/etc/config 
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ZIP_Router.linux $(1)/usr/bin/zipgateway
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zipgateway $(1)/usr/bin/zipgateway
	$(INSTALL_BIN)  ./files/zipgateway.init $(1)/etc/init.d/zipgateway
	$(INSTALL_BIN) ./files/zipgateway.tun $(1)/etc/zipgateway.tun
	$(INSTALL_CONF) ./files/zipgateway.config $(1)/etc/config/zipgateway

	sed -i "s/@NAME@/zipgateway/g" $(1)/etc/config/zipgateway
	sed -i "s/@NAME@/zipgateway/g" $(1)/etc/init.d/zipgateway

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin $(1)/usr/lib/lua/luci/model/cbi/zipgateway
	$(INSTALL_BIN) files/luci/controller/admin/zipgateway.lua $(1)/usr/lib/lua/luci/controller/admin/zipgateway.lua
	$(INSTALL_BIN) files/luci/model/cbi/zipgateway/setup.lua $(1)/usr/lib/lua/luci/model/cbi/zipgateway/setup.lua
	sed -i "s/@NAME@/zipgateway/g" $(1)/usr/lib/lua/luci/controller/admin/zipgateway.lua
	sed -i "s/@NAME@/zipgateway/g" $(1)/usr/lib/lua/luci/model/cbi/zipgateway/setup.lua

	$(INSTALL_CONF) ./files/ZIPR.x509_1024.pem $(1)/etc/zipgateway.x509_1024.pem
	$(INSTALL_CONF) ./files/ZIPR.key_1024.pem $(1)/etc/zipgateway.key_1024.pem
	$(INSTALL_CONF) ./files/Portal.ca_x509.pem $(1)/etc/zipgateway.CA_x509.pem
endef

#define Package/zipgateway_demo/install
#	$(INSTALL_BIN) ./files/demorouter.postinst $(1)/CONTROL/postinst
#endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,zipgateway,+libopenssl))
$(eval $(call BuildPackage,zipgateway_demo))


