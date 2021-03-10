# add include path directories
INCLUDE_DIRS += \
    -I${MSGTOOLS_CODEGEN_DIR}/..                 \
    -I$(UCPLATFORM)/simulink                     \
    -I.                                          \
    -I$(SIMULINK_MODEL_NAME)_grt_rtw/            \


# we need the below include dirs for these files:
SIMULINK_HEADER_FILES := \
    /usr/local/MATLAB/R2020b/rtw/c/src/rt_logging.h                 \
    /usr/local/MATLAB/R2020b/rtw/c/src/rt_mxclassid.h               \
    /usr/local/MATLAB/R2020b/simulink/include/rtw_continuous.h      \
    /usr/local/MATLAB/R2020b/simulink/include/rtw_extmode.h         \
    /usr/local/MATLAB/R2020b/simulink/include/rtw_matlogging.h      \
    /usr/local/MATLAB/R2020b/simulink/include/rtw_solver.h          \
    /usr/local/MATLAB/R2020b/simulink/include/simstruc_types.h      \
    /usr/local/MATLAB/R2020b/simulink/include/sl_sample_time_defs.h \
    /usr/local/MATLAB/R2020b/simulink/include/sl_types_def.h        \
    /usr/local/MATLAB/R2020b/simulink/include/sysran_types.h        \
    /usr/local/MATLAB/R2020b/extern/include/tmwtypes.h

INCLUDE_DIRS += \
    -I/usr/local/MATLAB/R2020b/rtw/c/src/        \
    -I/usr/local/MATLAB/R2020b/extern/include/   \
    -I/usr/local/MATLAB/R2020b/simulink/include/

INCLUDE_DIRS += \
    -I${MSGTOOLS_CODEGEN_DIR} \
    -I$(UCPLATFORM)/msg/


SRC += \
    ucplatform/simulink/simulink_message_client.cpp \
    ucplatform/simulink/simulink_message_interface.cpp \
    $(SIMULINK_MODEL_NAME)_grt_rtw/$(SIMULINK_MODEL_NAME).cpp \
    $(SIMULINK_MODEL_NAME)_grt_rtw/rt_nonfinite.cpp \
    $(SIMULINK_MODEL_NAME)_grt_rtw/rtGetNaN.cpp \
    $(SIMULINK_MODEL_NAME)_grt_rtw/rtGetInf.cpp \
    $(SIMULINK_MODEL_NAME)_grt_rtw/$(SIMULINK_MODEL_NAME)_data.cpp \

$(OBJ_DIR)/$(SIMULINK_MODEL_NAME)_grt_rtw/$(SIMULINK_MODEL_NAME).o: FLAGS += -Wno-unused-value

LDFLAGS += -Wl,-z,muldefs

FLAGS += -DMAT_FILE=0
