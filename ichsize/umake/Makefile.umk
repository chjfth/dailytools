# template file for use with pattern1.mks, concise version, [GnumakeUniproc v1.0]
# ! Be sure to replace every occurrence of <angle-brackets> to be your
# actual content, or [ at least comment them out if some of them are
# not required] .

ifdef gmu_DIR_GNUMAKEUNIPROC
  include $(gmu_DIR_GNUMAKEUNIPROC)/pattern1-concise-header.mki
else
  $(error gmu_DIR_GNUMAKEUNIPROC is not defined, You have to set up GnumakeUniproc environment to use this Makefile)
endif

############################################################################
# GMU-var: gmu_PRJ_NAME
# 	Define the project name(prj-name for short).
# project name for a project(precisely, after decoration
# into gmu_G_Final_PrjName) should be unique 
# across the whole build process of one GnumakeUniproc run. Prj-name
# conflict will be detected by GMU if two or more projects have the same
# gmu_G_Final_PrjName.
############################################################################
ifndef gmu_PRJ_NAME
  gmu_PRJ_NAME = ichsize-chj
endif

############################################################################
# GMU-var: gmp_bc_IMAGE_TYPE
#	Define the "image-type" to generated for this project.
# Common image-types are: LIB, DLL and EXE.
############################################################################
gmp_bc_IMAGE_TYPE = EXE

############################################################################
# Compiler specific settings below.
############################################################################
ifeq ($(call gmuf_IsWordInSet,$(gmp_COMPILER_ID),msvc wince),1)
  # msvc, wince compiler 
  gmp_bc_CRT_STATIC_LIB = 1
	# 1: Use static CRT lib (compiler option /MT or /MTd)
	# (null): use DLL CRT lib (compiler option /MD or /MDd)

  # gmp_CFLAGS_NON_CODE_GEN += /W3
  
  # gmp_u_list_PLUGIN_TO_LOAD += PI_winres_msvc       # compile Windows resource(.rc)
    # gmi_RCVC_up_list_WINRC_FROM_ROOT = <.rc-filepath>
	#	This is a required parameter for PI_winres_msvc plugin, telling where is 
	#	the Windows resource script file, filepath relative to $(gmu_ud_SRC_ROOT).
  # gmp_u_list_PLUGIN_TO_LOAD += PI_use_pch_msvc      # use precompile header
    # gmi_PCHVC_up_HEADER_FOR_PCH_FROM_ROOT = StdAfx.h
	#	This tells the .H file used for precompiled header(passed as /Yc 's argument)
  # gmp_u_list_PLUGIN_TO_LOAD += PI_windll_from_lib   # make dll from lib
  # gmp_u_list_PLUGIN_TO_LOAD += PI_sync_debug_info

else ifeq ($(call gmuf_IsWordInSet,$(gmp_COMPILER_ID),linuxgcc),1)
  # GCC compiling linux program

  # gmp_u_list_PLUGIN_TO_LOAD += PI_gcc_strip_lo      # strip output executable
  # gmp_u_list_PLUGIN_TO_LOAD += PI_use_pch_gcc       # use precompile header
  
else # ... some other gmp_COMPILER_ID processing ...

endif

############################################################################
# GMU-var: gmp_u_list_PLUGIN_TO_LOAD
#	Designate plugins to load in this make-var, multiple plugin names separated
# by spaces. Plugins are searched from $(gmp_ud_list_CUSTOM_MKI) and then
# from $(gmu_DIR_GNUMAKEUNIPROC)/pattern1cfg/plugins .
############################################################################
gmp_u_list_PLUGIN_TO_LOAD += 
	# PI_sync_devoutput PI_always_compile PI_file_precopy


############################################################################
# GMU-var: gmu_uf_LINK_OUTPUT
#	Define the link ouput filename for this project.
# Decoration may be applied to this name which results in $(gmu_Gf_Final_LinkOutput).
# Using function gmpf_MakeExeName to construct output name makes the name more portable.
############################################################################
gmu_uf_LINK_OUTPUT = $(call gmpf_MakeExeName,ichsize)
gmp_DECO_UNICODE_FOR_LINK_OUTPUT = 
	#Set this to 1 if you want a _U suffix on your output-image when gmp_bc_UNICODE=1
gmp_DECO_DEBUG_FOR_LINK_OUTPUT = 
	#Set this to 1 if you want a _D suffix on your output-image when gmp_bc_DEBUG=1

############################################################################
# GMU-var: gmu_ud_SRC_ROOT
#	Define the root dir where all your source files(.c, .cpp, not .h) reside in.
# You can use a relative dir here, but note, it is relative to make-var $(CURDIR),
# not to this mki file. Normally, $(CURDIR) is your start-up makefile directory.
############################################################################
gmu_ud_SRC_ROOT = .

############################################################################
# GMU-var: gmu_up_list_SRC_FROM_ROOT
#	Define your source files to compile, source files(dir prefix allowed) separated by spaces or tabs.
# You MUST use relative paths here, which are relative to $(gmu_ud_SRC_ROOT), 
# however, any of the resulting absolute path should not go up beyond $(gmu_ud_SRC_ROOT),
# e.g.
# * You cannot use ``../myfunc.c'', which actually resides in $(gmu_ud_SRC_ROOT)'s parent dir.
# * You can use ``subdir1/../myfunc.c'' or ``subdir1/subdir2/../myfunc.c'', which are 
#	still in $(gmu_ud_SRC_ROOT) or $(gmu_ud_SRC_ROOT)'s subdir.
############################################################################
gmu_up_list_SRC_FROM_ROOT += ichsize.cpp

############################################################################
# GMU-var: gmu_ud_list_PRJ_HEADER
#	Define header file dirs, multiple dirs should be separated by spaces or tabs.
# You can use relative dirs here, but note, they are relative to make-var $(CURDIR),
# not to this mki file.
#	This var serves two purposes:
#	1. They'll be passed to the compiler through ``-I <dir1> -I <dir2> ...'' .
#	2. They'll be passed to `$(gmu_GCC_M_PREFIX)gcc -M' in favor of generating hdepend files.
############################################################################
gmu_ud_list_PRJ_HEADER += # <dir1> <dir2> ...
	# Don't write ``-I'' in this var; -I will be prependded automatically.

gmp_CFLAGS_INCLUDES += # $(addprefix -I,<dir1> <dir2> ...)
	# (normally, you don't need this)
	# those -I xxx options (in case you don't want to list them in gmu_ud_list_PRJ_HEADER)
	# You must explicitly write ``-I'' before every dir in gmp_CFLAGS_INCLUDES

############################################################################
# GMU-vars: gmp_CFLAGS_<XXX>
#	Set specific compiler options for this project in the following vars:
# gmp_CFLAGS_CODE_GEN_ALL:
#	Any options that affect the generated binary code should be listed in it.
#	Consider gmp_CFLAGS_CODE_GEN_ALL as your accustomed CFLAGS.
# gmp_CFLAGS_NON_CODE_GEN:
#	Any options that do not affect the generated binary code should be listed in it.
############################################################################

gmp_CFLAGS_CODE_GEN_ALL +=
  # You may want to dispense options for gmp_CFLAGS_CODE_GEN_ALL into following 2 categories:
  gmp_CFLAGS_MACRO_DEFS += $(gmp_CFLAGS_MACRO_DEFS_preset) # append other macro defines, e.g. -Dxxx=yyy
  gmp_CFLAGS_CODE_GEN += $(gmp_CFLAGS_CODE_GEN_preset) # append other code generation related options

gmp_CFLAGS_NON_CODE_GEN += $(gmp_CFLAGS_NON_CODE_GEN_preset)

# If some compiler options(.e.g -Dxxx ) must be passed to `gcc -M' command, set them here:
# (Note: Those include-dir options(-I xxx) don't have to be set here.)
gmu_u_GCC_M_OPTIONS +=


############################################################################
# GMU-vars: gmp_LINKFLAGS
#	Set linker options for this project in this var.
############################################################################
gmp_LINKFLAGS += $(gmp_LINKFLAGS_preset)
  # There are some other make-vars for easy setting linker flags:
  gmp_ud_list_EXTRALIB += 
  	# list library paths here, compiler-config-mki will help you prepend(for example) 
  	# * -L for linuxgcc
  	# * /LIBPATH: for msvc

############################################################################
#	List extra objs or libs to link below, they'll be combined into gmp_ALL_TO_LINK
############################################################################
gmu_up_list_EXTRA_OBJ_TO_LINK +=
	# Extra object files to link

gmp_bc_EXTRALIBS += # $(call gmpf_MakeLibName,<lib-list>)
	# Extra libs to link
	#
	# $(call gmpf_MakeLibName,foo) will become(for example):
	# * libfoo.a         for linuxgcc & mingw
	# * foo.lib          for msvc

gmp_bc_SYSLIBS += # $(call gmpf_MakeUpSysLibsToLink,<sys-lib-list>)
	# List system libs in this make-var. 
	# If more than one sys-lib are used, separate them with spaces.
	#	note: The behavior of gmp_bc_SYSLIBS is exactly the same as gmp_bc_EXTRALIBS.
	# It is used just to imply that they are system libs(provided by compiler of OS).
	#
	# The reason for using gmpf_MakeUpSysLibsToLink: 
	# $(call gmpf_MakeUpSysLibsToLink,comfun) will become(for example):
	# * -lcomfun         for linuxgcc & mingw
	# * comfun.lib       for msvc


#---------------------------------------------------------------------------
# Sub-projects definition starts here. 
# Library(static lib or DLL) projects may well be your sub-projects for this EXE project
#---------------------------------------------------------------------------
############################################################################
# GMU-var: gmu_uv_list_SUBPRJ_TO_LINK (outdated)
#	This var tells which projects are sub-project for this project(parent project), 
# and, for sub-projects listed in this var, their output-images will automatically 
# become link components for parent project.
#
# GMU-var: gmu_uv_list_SUBPRJ_LINK_ALL_LEVEL (recommended)
#	This one is the superset of gmu_uv_list_SUBPRJ_TO_LINK. It's powerful.
#	Not only output-images of sub-projects explicitly listed in gmu_uv_list_SUBPRJ_LINK_ALL_LEVEL 
# will be linked by this project, but also sub-projects' of those explictly listed ones 
# -- any level of sub-projects can be linked. Anyhow, there are two factors that 
# determines whether deeper projects' outputs will be automatically linked:
# 1. a deeper project must claim himself a link-component(gmu_I_AM_A_LINK_COMPONENT=1).
# You know, if you use pattern1.mks, LIB image-type has gmu_I_AM_A_LINK_COMPONENT=1 
# already defined.
# 2. For your sub-project(called SP), SP must define his sub-projects(called SSPs)
# in gmu_uv_list_SUBPRJ_LINK_FOR_PARENT in order for SSPs to be linked automatically
# by you.
############################################################################

#gmu_uv_list_SUBPRJ_TO_LINK += # <subprj1-refname> <subprj2-refname> ...
# or
gmu_uv_list_SUBPRJ_LINK_ALL_LEVEL += # <subprj1-refname> <subprj2-refname> ...

######## Details of your sub-projects go here: ########

<subprj1-refname>_Makefile = # where the sub-project's makefile resides
<subprj1-refname>_MakeVarDefines = # command line options passed to sub-make
# If the sub-project's makefiles are to be checked-out from some repository, assign it here.
# >>>> For SVN checkout
<subprj1-refname>_svnUrl_forMakefiles = # SVN Url for the checkout
	# Hint: $(call gmuf_MakeSvnRepositoryPath,<SVN-path-prefix-var>,<remaining-repository-path>)
<subprj1-refname>_svnLocalDir_forMakefiles = # Local dir corresponding to the whole SVN URL
<subprj1-refname>_svnRev_forMakefiles = # Revision to use(string after svn's -r switch)
# <<<< For SVN checkout
# >>>> For CVS checkout
<subprj1-refname>_cvsRoot_forMakefiles = # CVSROOT for the checkout
	# Hint: $(call gmuf_MakeCvsRepositoryPath,<CVS-server-var>,<repository-path-on-CVS-server>)
<subprj1-refname>_cvsModuleDir_forMakefiles = # CVS module name or module dir to checkout
<subprj1-refname>_cvsLocalDir_forMakefiles = # Local dir corresponding to CVSROOT
<subprj1-refname>_cvsTag_forMakefiles = # Tag or branch to checkout(string after cvs's -r switch)
# <<<< For CVS checkout

# NOTE for novice: Don't try to make sub-projects' output libs listed in gmp_ALL_TO_LINK,
# those libs will be listed in gmp_ALL_TO_LINK automatically.


# ----------- below for plugins -----------

# If you use plugin PI_sync_devoutput, you may want to set:
# gmi_SYDO_ud_SYNC_HEADER_FROM =
	# The dir that your headers reside in. ( relative to $(CURDIR) )

# gmi_SYDO_up_list_SYNC_HEADER =
	# List filepaths of the header files to sync. ( relative to $(gmi_SYDO_ud_SYNC_HEADER_FROM) )

# gmi_SYDO_ud_SYNC_HEADER_TO = # (suggested to set as env-var or by parent project)
	# Where the headers will be sync-ed to.

# gmi_SYDO_ud_SYNC_EXE_TO = # (suggested to set as env-var or by parent project)



include $(gmu_DIR_GNUMAKEUNIPROC)/pattern1.mks

