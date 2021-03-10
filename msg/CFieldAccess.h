#ifndef __FIELD_ACCESS_H__
#define __FIELD_ACCESS_H__

#ifndef INLINE
#define INLINE
#endif
#ifndef UNUSED
#define UNUSED (void)
#endif

#define Get_int8_t(location) *(int8_t*)(location)
#define Get_int16_t(location) *(int16_t*)(location)
#define Get_int32_t(location) *(int32_t*)(location)
#define Get_int64_t(location) *(int64_t*)(location)
#define Get_uint8_t(location) *(uint8_t*)(location)
#define Get_uint16_t(location) *(uint16_t*)(location)
#define Get_uint32_t(location) *(uint32_t*)(location)
#define Get_uint64_t(location) *(uint64_t*)(location)
#define Get_float(location) *(float*)(location)
#define Get_double(location) *(double*)(location)

#define Set_int8_t(location, value) do { *(int8_t*)(location) = value; } while(0)
#define Set_int16_t(location, value) do { *(int16_t*)(location) = value; } while(0)
#define Set_int32_t(location, value) do { *(int32_t*)(location) = value; } while(0)
#define Set_int64_t(location, value) do { *(int64_t*)(location) = value; } while(0)
#define Set_uint8_t(location, value) do { *(uint8_t*)(location) = value; } while(0)
#define Set_uint16_t(location, value) do { *(uint16_t*)(location) = value; } while(0)
#define Set_uint32_t(location, value) do { *(uint32_t*)(location) = value; } while(0)
#define Set_uint64_t(location, value) do { *(uint64_t*)(location) = value; } while(0)
#define Set_float(location, value) do { *(float*)(location) = value; } while(0)
#define Set_double(location, value) do { *(double*)(location) = value; } while(0)

#endif
