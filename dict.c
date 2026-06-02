#define BUILDING_DICT_DLL
#include "dict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

int convert_struct_to_json(const DictData *data, char *out_json, int max_len) {
    if (data == NULL || out_json == NULL) return -1;
    int written = snprintf(out_json, max_len, 
        "{\"auth_code\":\"%s\",\"dict_type\":%d,\"keyword\":\"%s\",\"client_ip\":\"%s\",\"client_port\":%d}", 
        data->auth_code, data->dict_type, data->keyword, data->client_ip, data->client_port);
    return (written > 0 && written < max_len) ? 0 : -2;
}

int convert_json_to_struct(const char *in_json, DictData *out_data) {
    if (in_json == NULL || out_data == NULL) return -1;
    
    const char *start = strstr(in_json, "\"definition\"");
    if (start == NULL) start = strstr(in_json, "\"meaning\"");
    if (start == NULL) start = strstr(in_json, "\"item\"");
    
    if (start != NULL) {
        start = strchr(start, ':');
        if (start != NULL) {
            start = strchr(start, '"');
            if (start != NULL) {
                start++;
                const char *end = strchr(start, '"');
                if (end != NULL) {
                    size_t len = end - start;
                    if (len >= sizeof(out_data->definition)) len = sizeof(out_data->definition) - 1;
                    strncpy(out_data->definition, start, len);
                    out_data->definition[len] = '\0';
                    return 0;
                }
            }
        }
    }
    
    strncpy(out_data->definition, "API 데이터 추출 불일치 (원보 데이터 참고)", sizeof(out_data->definition) - 1);
    return 0;
}

int report_to_server(const DictData *data, const char *server_ip, int server_port) {
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return -1;
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
#if defined(_WIN32) || defined(_WIN64)
        WSACleanup();
#endif
        return -2;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    
#if defined(_WIN32) || defined(_WIN64)
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
#else
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);
#endif

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
#if defined(_WIN32) || defined(_WIN64)
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return -3;
    }

    char report_payload[65536 + 1024];
    memset(report_payload, 0, sizeof(report_payload));
    
    snprintf(report_payload, sizeof(report_payload),
        "AUTH:%s\nIP:%s\nPORT:%d\nTYPE:%d\nKEY:%s\nDEF:%s\nJSON:%s\n",
        data->auth_code, data->client_ip, data->client_port,
        data->dict_type, data->keyword, data->definition, data->raw_json);

    send(sock, report_payload, (int)strlen(report_payload), 0);

#if defined(_WIN32) || defined(_WIN64)
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}

int execute_dict_dll(DictData *io_data, const char *server_ip, int server_port) {
    if (io_data == NULL) return -1;
    if (strcmp(io_data->auth_code, "TEST999") != 0) {
        strcpy(io_data->definition, "인증 코드 검증 실패 (동작 거부)");
        return -9;
    }

    char command[2048] = {0,};
    char line_buffer[4096] = {0,};
    const char *base_url = "";
    const char *api_key = "YOUR_REAL_API_KEY";

    if (io_data->dict_type == 1) {
        base_url = "https://korean.go.kr";
    } else if (io_data->dict_type == 2) {
        base_url = "https://data.go.kr";
    } else {
        return -3;
    }

    snprintf(command, sizeof(command), 
             "curl -s -L \"%s?key=%s&q=%s&req_type=json\" > dll_temp.txt", 
             base_url, api_key, io_data->keyword);

    int sys_status = system(command);
    if (sys_status != 0) return -4;

    FILE *fp = fopen("dll_temp.txt", "r");
    if (fp == NULL) return -5;

    io_data->raw_json[0] = '\0';
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
        if (strlen(io_data->raw_json) + strlen(line_buffer) < sizeof(io_data->raw_json) - 1) {
            strcat(io_data->raw_json, line_buffer);
        } else {
            break;
        }
    }
    fclose(fp);

#if defined(_WIN32) || defined(_WIN64)
    system("del /f /q dll_temp.txt >nul 2>&1");
#else
    system("rm -f dll_temp.txt > /dev/null 2>&1");
#endif

    convert_json_to_struct(io_data->raw_json, io_data);
    report_to_server(io_data, server_ip, server_port);

    return 0;
}
