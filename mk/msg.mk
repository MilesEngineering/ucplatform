MSG_INC_DIR=msg
MSGTOOLS_CODEGEN_DIR=../obj/CodeGenerator/Cpp/

INCLUDE_DIRS += -I${MSGTOOLS_CODEGEN_DIR}

INCLUDE_DIRS += -I${MSG_INC_DIR}

SRC += msg/message_client.cpp msg/message.cpp  msg/message_pool.cpp  msg/message_queue.cpp
