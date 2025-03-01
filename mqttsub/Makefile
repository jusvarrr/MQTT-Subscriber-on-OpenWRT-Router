include $(TOPDIR)/rules.mk

PKG_NAME:=mqttsub
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=MQTT Subscriber
	DEPENDS:=+cJSON +libuci +USE_UCLIBC:argp-standalone +libcurl +libmosquitto +lsqlite3
endef

define Package/$(PKG_NAME)/description
	A lightweight MQTT Subscriber package.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) ./files/readlog.lua $(1)/usr/bin
	$(INSTALL_CONF) ./files/subscriber $(1)/etc/config/subscriber
endef

$(eval $(call BuildPackage,mqttsub))
