package=gmp
$(package)_version=6.3.0
# ftp.gnu.org refuses connections from GitHub Actions runners; the sha256 below pins the file whatever the mirror.
$(package)_download_path=https://mirrors.kernel.org/gnu/gmp
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=e56fd59d76810932a0555aa15a14b61c16bed66110d3c75cc2ac49ddaa9ab24c

define $(package)_set_vars
$(package)_config_opts=--disable-shared
$(package)_config_opts_mingw32=--enable-mingw
$(package)_config_opts_linux=--with-pic
$(package)_config_opts_darwin=--with-pic
# ld64.lld rejects the short branch relocations in GMP's x86_64 assembly; use GMP's C code on macOS.
$(package)_config_opts_darwin+=--disable-assembly
# GMP 6.3.0's configure probes fail under GCC 15's default C23 mode.
$(package)_cflags+=-std=gnu17
# GMP builds table generators with the build compiler; use depends' native compiler (as sqlite.mk does).
$(package)_config_env := CC_FOR_BUILD="$$(build_CC)"
# GMP's compiler probe links with CC and CFLAGS only, so pass along whatever linker selection depends put in the Darwin LDFLAGS (lld when cross-compiling, nothing natively).
$(package)_cflags_darwin+=$(filter -fuse-ld=%,$(darwin_LDFLAGS))
endef

# On OpenBSD the generated libtool runs "ranlib -t" after install, which llvm-ranlib rejects; drop the -t (no-op elsewhere).
define $(package)_config_cmds
  $($(package)_autoconf) && \
  sed -i.old 's/RANLIB -t /RANLIB /' libtool
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

