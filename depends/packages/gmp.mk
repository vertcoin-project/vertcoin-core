package=gmp
$(package)_version=6.3.0
$(package)_download_path=https://ftp.gnu.org/gnu/gmp
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=e56fd59d76810932a0555aa15a14b61c16bed66110d3c75cc2ac49ddaa9ab24c

define $(package)_set_vars
$(package)_config_opts=--disable-shared
$(package)_config_opts_mingw32=--enable-mingw
$(package)_config_opts_linux=--with-pic
$(package)_config_opts_darwin=--with-pic
# GMP 6.3.0's configure probes fail under GCC 15's default C23 mode.
$(package)_cflags+=-std=gnu17
# GMP builds table generators with the build compiler; use depends' native compiler (as sqlite.mk does).
$(package)_config_env := CC_FOR_BUILD="$$(build_CC)"
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

