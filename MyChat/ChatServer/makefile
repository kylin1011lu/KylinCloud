TARGET   	= mychatserver
INSTALL_DIR = bins

SOURCES	 = 	src/link.c \
			src/mythread.c \
			src/server.c \
			protobuf-lite/google/protobuf/io/coded_stream.cc \
			protobuf-lite/google/protobuf/stubs/common.cc \
			protobuf-lite/google/protobuf/extension_set.cc \
			protobuf-lite/google/protobuf/generated_message_util.cc \
			protobuf-lite/google/protobuf/message_lite.cc \
			protobuf-lite/google/protobuf/stubs/once.cc \
			protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.cc \
			protobuf-lite/google/protobuf/repeated_field.cc \
			protobuf-lite/google/protobuf/wire_format_lite.cc \
			protobuf-lite/google/protobuf/io/zero_copy_stream.cc \
			protobuf-lite/google/protobuf/io/zero_copy_stream_impl_lite.cc \
			protobuf-lite/google/protobuf/stubs/stringprintf.cc \
			protobuf/common.pb.cc 


ROOT_DIR = .


CXXFLAGS += \
	-I. \
	-I$(ROOT_DIR)/include \
	-I./protobuf-lite \
	-I./protobuf \
	-m64 \
	-D_REENTRANT 

LIBS += \
	-lpthread 

CPPFLAGS += \
	-W \
	-Wall \
	-g \
	-std=c++11

OBJS_DIR = objs
DEPS_DIR = deps
DEPS = $(DEPS_DIR)/$(TARGET).d

ALL_OBJ_FILES:=$(patsubst %.cc, $(OBJS_DIR)/%.o, $(notdir $(SOURCES)))
ALL_OBJ_FILES:=$(patsubst %.cpp, $(OBJS_DIR)/%.o, $(ALL_OBJ_FILES))
ALL_OBJ_FILES:=$(patsubst %.c, $(OBJS_DIR)/%.o, $(ALL_OBJ_FILES))

all: remove_target $(TARGET)

$(TARGET):$(ALL_OBJ_FILES)
	@echo Linking
	@$(CXX) -o $@ $(ALL_OBJ_FILES) $(LIBS)
	@echo "  Built executable [ $@ ]"
	@echo
	@mkdir -p $(INSTALL_DIR)
	@mv $(TARGET) $(INSTALL_DIR)

$(OBJS_DIR)/%.o:
	@echo Compiling $<
	@$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<


remove_target:
	@rm -fr $(TARGET) $(INSTALL_DIR)/$(TARGET)

dep: 
	@echo "Generating dependencies..."
	@mkdir -p $(DEPS_DIR)
	@mkdir -p $(OBJS_DIR)
	@$(CXX) -MM $(CXXFLAGS) $(CPPFLAGS) $(SOURCES) | sed "/^[_a-zA-Z0-9.]*\.o:/s/^/\.\/$(OBJS_DIR)\//" > $(DEPS)


clean:
	@echo "Cleaning up..."$(ALL_OBJ_FILES)
	@rm -fr $(TARGET) $(DEPS_DIR) $(OBJS_DIR) $(INSTALL_DIR)/$(TARGET)

.PHONY: all clean tag dep install framework shard test

-include $(DEPS)