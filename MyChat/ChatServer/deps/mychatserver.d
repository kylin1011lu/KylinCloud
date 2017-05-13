./objs/link.o: src/link.c include/link.h include/mythread.h include/mythread.h
./objs/mythread.o: src/mythread.c include/mythread.h include/link.h \
 include/mythread.h include/dis.h include/protocol.h \
 protobuf-lite/google/protobuf/message_lite.h \
 protobuf-lite/google/protobuf/stubs/common.h protobuf/common.pb.h \
 protobuf-lite/google/protobuf/generated_message_util.h \
 protobuf-lite/google/protobuf/repeated_field.h \
 protobuf-lite/google/protobuf/stubs/type_traits.h \
 protobuf-lite/google/protobuf/stubs/template_util.h \
 protobuf-lite/google/protobuf/extension_set.h
./objs/server.o: src/server.c include/mythread.h include/link.h \
 include/mythread.h include/dis.h
./objs/coded_stream.o: protobuf-lite/google/protobuf/io/coded_stream.cc \
 protobuf-lite/google/protobuf/io/coded_stream_inl.h \
 protobuf-lite/google/protobuf/io/coded_stream.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/stubs/stl_util.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream.h
./objs/common.o: protobuf-lite/google/protobuf/stubs/common.cc \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/stubs/once.h \
 protobuf-lite/google/protobuf/stubs/atomicops.h \
 protobuf-lite/google/protobuf/stubs/platform_macros.h \
 protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.h \
 protobuf-lite/config.h
./objs/extension_set.o: protobuf-lite/google/protobuf/extension_set.cc \
 protobuf-lite/google/protobuf/stubs/hash.h \
 protobuf-lite/google/protobuf/stubs/common.h protobuf-lite/config.h \
 protobuf-lite/google/protobuf/stubs/once.h \
 protobuf-lite/google/protobuf/stubs/atomicops.h \
 protobuf-lite/google/protobuf/stubs/platform_macros.h \
 protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.h \
 protobuf-lite/google/protobuf/extension_set.h \
 protobuf-lite/google/protobuf/message_lite.h \
 protobuf-lite/google/protobuf/io/coded_stream.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream_impl.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream_impl_lite.h \
 protobuf-lite/google/protobuf/wire_format_lite_inl.h \
 protobuf-lite/google/protobuf/repeated_field.h \
 protobuf-lite/google/protobuf/stubs/type_traits.h \
 protobuf-lite/google/protobuf/stubs/template_util.h \
 protobuf-lite/google/protobuf/generated_message_util.h \
 protobuf-lite/google/protobuf/wire_format_lite.h \
 protobuf-lite/google/protobuf/stubs/map-util.h
./objs/generated_message_util.o: \
 protobuf-lite/google/protobuf/generated_message_util.cc \
 protobuf-lite/google/protobuf/generated_message_util.h \
 protobuf-lite/google/protobuf/stubs/common.h
./objs/message_lite.o: protobuf-lite/google/protobuf/message_lite.cc \
 protobuf-lite/google/protobuf/message_lite.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/io/coded_stream.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream_impl_lite.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream.h \
 protobuf-lite/google/protobuf/stubs/stl_util.h
./objs/once.o: protobuf-lite/google/protobuf/stubs/once.cc \
 protobuf-lite/google/protobuf/stubs/once.h \
 protobuf-lite/google/protobuf/stubs/atomicops.h \
 protobuf-lite/google/protobuf/stubs/platform_macros.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.h
./objs/atomicops_internals_x86_gcc.o: \
 protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.cc \
 protobuf-lite/google/protobuf/stubs/atomicops.h \
 protobuf-lite/google/protobuf/stubs/platform_macros.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.h
./objs/repeated_field.o: protobuf-lite/google/protobuf/repeated_field.cc \
 protobuf-lite/google/protobuf/repeated_field.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/stubs/type_traits.h \
 protobuf-lite/google/protobuf/stubs/template_util.h \
 protobuf-lite/google/protobuf/generated_message_util.h \
 protobuf-lite/google/protobuf/message_lite.h
./objs/wire_format_lite.o: protobuf-lite/google/protobuf/wire_format_lite.cc \
 protobuf-lite/google/protobuf/wire_format_lite_inl.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/message_lite.h \
 protobuf-lite/google/protobuf/repeated_field.h \
 protobuf-lite/google/protobuf/stubs/type_traits.h \
 protobuf-lite/google/protobuf/stubs/template_util.h \
 protobuf-lite/google/protobuf/generated_message_util.h \
 protobuf-lite/google/protobuf/wire_format_lite.h \
 protobuf-lite/google/protobuf/io/coded_stream.h \
 protobuf-lite/google/protobuf/io/coded_stream_inl.h \
 protobuf-lite/google/protobuf/stubs/stl_util.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream_impl_lite.h
./objs/zero_copy_stream.o: protobuf-lite/google/protobuf/io/zero_copy_stream.cc \
 protobuf-lite/google/protobuf/io/zero_copy_stream.h \
 protobuf-lite/google/protobuf/stubs/common.h
./objs/zero_copy_stream_impl_lite.o: \
 protobuf-lite/google/protobuf/io/zero_copy_stream_impl_lite.cc \
 protobuf-lite/google/protobuf/io/zero_copy_stream_impl_lite.h \
 protobuf-lite/google/protobuf/io/zero_copy_stream.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/stubs/stl_util.h
./objs/stringprintf.o: protobuf-lite/google/protobuf/stubs/stringprintf.cc \
 protobuf-lite/google/protobuf/stubs/stringprintf.h \
 protobuf-lite/google/protobuf/stubs/common.h
./objs/common.pb.o: protobuf/common.pb.cc protobuf/common.pb.h \
 protobuf-lite/google/protobuf/stubs/common.h \
 protobuf-lite/google/protobuf/generated_message_util.h \
 protobuf-lite/google/protobuf/message_lite.h \
 protobuf-lite/google/protobuf/repeated_field.h \
 protobuf-lite/google/protobuf/stubs/type_traits.h \
 protobuf-lite/google/protobuf/stubs/template_util.h \
 protobuf-lite/google/protobuf/extension_set.h \
 protobuf-lite/google/protobuf/stubs/once.h \
 protobuf-lite/google/protobuf/stubs/atomicops.h \
 protobuf-lite/google/protobuf/stubs/platform_macros.h \
 protobuf-lite/google/protobuf/stubs/atomicops_internals_x86_gcc.h \
 protobuf-lite/google/protobuf/io/coded_stream.h \
 protobuf-lite/google/protobuf/wire_format_lite_inl.h \
 protobuf-lite/google/protobuf/wire_format_lite.h
