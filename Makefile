# For build all eyre_turing_lib and test.

# Input windows or linux.
SYSTEM = windows

# RELEASE_MODE input static or shared.
RELEASE_MODE = static

# LIBNEED is which lib you want to build.
ifeq ($(RELEASE_MODE),static)
LIBNEED = network framework
else
LIBNEED = framework network
endif

# Input which system lib compile link need.
SYSTEM_LIB_LINK = -lpthread
ifeq ($(SYSTEM),windows)
SYSTEM_LIB_LINK += -lws2_32
endif

# Like input -m32 for compilation a 32bit lib.
COMPILE_OPTION = 

# RM is your delete file tool command name.
RM = rm -f

################################################################
# Don't change any of the following.

ifeq ($(SYSTEM),windows)
MAKE = mingw32-make
SHARED_LIB_SUFFIX = .dll
SHARED_LIB_PREFIX = 
STATIC_LIB_SUFFIX = .a
else
MAKE = make
SHARED_LIB_SUFFIX = .so
SHARED_LIB_PREFIX = lib
STATIC_LIB_SUFFIX = .a
endif

TARGET = test
OBJECT = test.o

MAKELIB_INC = $(patsubst %, -I%/inc/, $(LIBNEED))

ifeq ($(RELEASE_MODE),static)
MAKELIB_OBJ = $(foreach n, $(LIBNEED), $(n)/lib/eyre_$(n)$(STATIC_LIB_SUFFIX))
else
MAKELIB_OBJ = $(foreach n, $(LIBNEED), $(n)/lib/$(SHARED_LIB_PREFIX)eyre_$(n)$(SHARED_LIB_SUFFIX))
MAKELIB_LINK = $(foreach n, $(LIBNEED), -L$(n)/lib/ -Wl,-rpath=. -leyre_$(n))
endif

$(TARGET) : $(OBJECT) $(MAKELIB_OBJ)
ifeq ($(RELEASE_MODE),static)
	g++ $(COMPILE_OPTION) $^ $(MAKELIB_INC) $(SYSTEM_LIB_LINK) -o $@
else
	g++ $(COMPILE_OPTION) $(OBJECT) $(MAKELIB_LINK) $(MAKELIB_INC) -o $@
endif

%.o : %.cpp
	g++ -c $(COMPILE_OPTION) $< $(MAKELIB_INC) -o $@

%$(STATIC_LIB_SUFFIX) :
	cd $(dir $@) && $(MAKE) TARGET=$(notdir $@) COMPILE_OPTION=$(COMPILE_OPTION) SYSTEM=$(SYSTEM) static

%$(SHARED_LIB_SUFFIX) :
	cd $(dir $@) && $(MAKE) TARGET=$(notdir $@) COMPILE_OPTION=$(COMPILE_OPTION) SYSTEM=$(SYSTEM) shared

.PHONY clean:
clean :
	$(RM) $(OBJECT)
	$(foreach n, $(dir $(MAKELIB_OBJ)), $(RM) $(n)/*.o)

.PHONY remove-lib:
remove-lib :
	$(RM) $(MAKELIB_OBJ)
