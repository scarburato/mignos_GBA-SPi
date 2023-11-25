#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x92997ed8, "_printk" },
	{ 0x226383ed, "i2c_get_adapter" },
	{ 0x3e7874ab, "i2c_new_client_device" },
	{ 0x440cbd26, "i2c_register_driver" },
	{ 0xbf4bb060, "i2c_put_adapter" },
	{ 0x60a32ea9, "pm_power_off" },
	{ 0xbec3491, "i2c_transfer_buffer_flags" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x1d31a384, "i2c_unregister_device" },
	{ 0x424d82fe, "i2c_del_driver" },
	{ 0x8f80e6e5, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:power-GBASPi");

MODULE_INFO(srcversion, "F198961B7ADA3F4996AFB27");
