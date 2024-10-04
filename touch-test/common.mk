# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=screen m EGL GLESv1_CM freetype

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

INSTALLDIR = sbin
INSTALL_ROOT_nto = $(PROJECT_ROOT)/../../../install
USE_INSTALL_ROOT=1

