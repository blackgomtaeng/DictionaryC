#ifndef DICT_H
#define DICT_H

#if defined(_WIN32) || defined(_WIN64)
    #if defined(_CONSOLE)
        #define DICT_API
    #elif defined(BUILDING_DICT_DLL)
        #define DICT_API __declspec(dllexport)
    #else
        #define DICT_API __declspec(dllimport)
    #endif
#else
    #define DICT_API __attribute__((visibility("default")))
#endif

typedef struct {
    int dict_type;
    char keyword[128];
    char definition[4096];
    char raw_json[65536];
    char auth_code[32];
    char client_ip[32];
    int client_port;
} DictData;

DICT_API int convert_struct_to_json(const DictData *data, char *out_json, int max_len);
DICT_API int convert_json_to_struct(const char *in_json, DictData *out_data);
DICT_API int report_to_server(const DictData *data, const char *server_ip, int server_port);
DICT_API int execute_dict_dll(DictData *io_data, const char *server_ip, int server_port);

#endif
