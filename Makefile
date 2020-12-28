# For build all eyre_turing_lib and test.

# LIBNEED is which lib you want to build.
LIBNEED = framework

# RELEASE_MODE input static or shared.
RELEASE_MODE = static

# Like input -m32 for compilation a 32bit lib.
COMPILE_OPTION = 

# STATIC_LIB_SUFFIX is what suffix whth static lib.
STATIC_LIB_SUFFIX = .a

# SHARED_LIB_PREFIX is what prefix whth shared lib.
SHARED_LIB_PREFIX = 

# SHARED_LIB_SUFFIX is what suffix with shared lib.
SHARED_LIB_SUFFIX = .dll

# MAKE is your compilation tool command name.
MAKE = mingw32-make

# RM is your delete file tool command name.
RM = rm -f

################################################################
# Don't change any of the following.

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
	g++ $(COMPILE_OPTION) $^ $(MAKELIB_INC) -o $@
else
	g++ $(COMPILE_OPTION) $(OBJECT) $(MAKELIB_LINK) $(MAKELIB_INC) -o $@
endif

%.o : %.cpp
	g++ -c $(COMPILE_OPTION) $< $(MAKELIB_INC) -o $@

%$(STATIC_LIB_SUFFIX) :
	cd $(dir $@) && $(MAKE) TARGET=$(notdir $@) COMPILE_OPTION=$(COMPILE_OPTION) static

%$(SHARED_LIB_SUFFIX) :
	cd $(dir $@) && $(MAKE) TARGET=$(notdir $@) COMPILE_OPTION=$(COMPILE_OPTION) shared

.PHONY clean:
clean :
	$(RM) $(OBJECT)
	$(foreach n, $(dir $(MAKELIB_OBJ)), $(RM) $(n)/*.o)

.PHONY remove-lib:
remove-lib :
	$(RM) $(MAKELIB_OBJ)
